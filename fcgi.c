#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <stdlib.h>
#include <kcgi.h>

#define unused __attribute__((__unused__))

#define REQ_MEMBER(name) khttp_puts(&req, ",\n\t." #name " = ");\
		if (req.name) {\
			khttp_puts(&req, req.name);\
		} else {\
			khttp_puts(&req, "NULL");\
		}

__attribute__((__always_inline__))
static inline void
print_kreq_scheme (struct kreq *r)
{
	khttp_puts(r, ",\n\t.scheme = ");
	khttp_puts(r, kschemes[r->scheme]);
}

#define REQ_MEMBER_ARRAY(name) print_kpair_array(&req, #name, req.name, req.name ## z)

static const char kpairstates[3][10] = {
	[KPAIR_UNCHECKED] = "unchecked",
	[KPAIR_VALID]     = "valid\0\0\0\0",
	[KPAIR_INVALID]   = "invalid\0\0"
};

enum key {
	KEY_TEST = 0,
	KEY__MAX
};

static const struct kvalid keys[KEY__MAX] = {
	{ kvalid_string, "test" } /* KEY_TEST */
};

static void
print_kpair_array (struct kreq  *req,
                   const char   *var,
                   struct kpair *arr,
                   size_t        len)
{
	khttp_puts(req, ",\n\t.");
	khttp_puts(req, var);

	if (!arr) {
		khttp_puts(req, " = NULL");
		return;
	}

	if (!len) {
		khttp_puts(req, " = {}");
		return;
	}

	khttp_puts(req, " = {\n\t\t");
	khttp_puts(req, arr->key);
	khttp_puts(req, " = ");
	khttp_puts(req, arr->val);
	khttp_puts(req, " <");
	khttp_puts(req, kpairstates[arr->state]);
	khttp_putc(req, '>');

	while (--len) {
		++arr;
		khttp_puts(req, ",\n\t\t");
		khttp_puts(req, arr->key);
		khttp_puts(req, " = ");
		khttp_puts(req, arr->val);
		khttp_puts(req, " <");
		khttp_puts(req, kpairstates[arr->state]);
		khttp_putc(req, '>');
	}

	khttp_puts(req, "\n\t}");
}

int
main (unused int    argc,
      unused char **argv)
{
	struct kreq req;
	struct kfcgi *fcgi;

	if (KCGI_OK != khttp_fcgi_initx(&fcgi,
	                                kmimetypes, KMIME__MAX,
	                                keys, sizeof(keys) / sizeof(*keys), /*keys, keysz,*/
	                                ksuffixmap, KMIME_TEXT_HTML,
	                                NULL, 0, 0, /*pages, pagesz, defpage,*/
	                                NULL, NULL, /*arg, argfree,*/
	                                KREQ_DEBUG_READ_BODY,
	                                NULL /*kopts*/)) {
		return EXIT_SUCCESS;
	}

	while (KCGI_OK == khttp_fcgi_parse(fcgi, &req)) {
		khttp_head(&req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
		khttp_head(&req, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[KMIME_TEXT_PLAIN]);
		khttp_body(&req);
		khttp_puts(&req, "struct kreq req = {\n\t.method = KMETHOD_");
		khttp_puts(&req, kmethods[req.method]);
		print_kreq_scheme(&req);
		REQ_MEMBER(path);
		REQ_MEMBER(suffix);
		REQ_MEMBER(fullpath);
		REQ_MEMBER(pagename);
		REQ_MEMBER(remote);
		REQ_MEMBER(host);
		REQ_MEMBER(pname);
		REQ_MEMBER_ARRAY(cookies);
		khttp_puts(&req, ",\n\t.cookiemap = ");
		khttp_puts(&req, (req.cookiemap) ? "..." : "NULL");
		khttp_puts(&req, ",\n\t.cookienmap = ");
		khttp_puts(&req, (req.cookienmap) ? "..." : "NULL");
		REQ_MEMBER_ARRAY(fields);
		khttp_puts(&req, ",\n\t.fieldmap = ");
		khttp_puts(&req, (req.fieldmap) ? "..." : "NULL");
		khttp_puts(&req, ",\n\t.fieldnmap = ");
		khttp_puts(&req, (req.fieldnmap) ? "..." : "NULL");
		khttp_puts(&req, "\n};");
		khttp_free(&req);
	}

	khttp_fcgi_free(fcgi);
	return EXIT_SUCCESS;
}

#if 0
struct  kreq {
        struct khead             *reqmap[KREQU__MAX];
        struct khead             *reqs;
        size_t                    reqsz;
        enum kmethod              method;
        enum kauth                auth;
        struct khttpauth          rawauth;
        struct kpair             *cookies;
        size_t                    cookiesz;
        struct kpair            **cookiemap;
        struct kpair            **cookienmap;
        struct kpair             *fields;
        struct kpair            **fieldmap;
        struct kpair            **fieldnmap;
        size_t                    fieldsz;
        size_t                    mime;
        size_t                    page;
        enum kscheme              scheme;
        char                     *path;
        char                     *suffix;
        char                     *fullpath;
        char                     *pagename;
        char                     *remote;
        char                     *host;
        uint16_t                  port;
        struct kdata             *kdata;
        const struct kvalid      *keys;
        size_t                    keysz;
        char                     *pname;
        void                     *arg;
};
#endif
