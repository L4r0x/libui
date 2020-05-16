#include "uipriv_unix.h"

struct uiprivMem {
	const char *type;
	void *p;
};

typedef struct uiprivMem uiprivMem;

static GArray *allocations;

static int removeAllocEntry(uiprivMem mem)
{
	for (guint i = 0; i < allocations->len; ++i) {
		if (g_array_index(allocations, uiprivMem, i).p == mem.p) {
			g_array_remove_index_fast(allocations, i);
			return TRUE;
		}
	}
	return FALSE;
}

static void addAllocEntry(uiprivMem mem)
{
	g_array_append_val(allocations, mem);
}

void uiprivInitAlloc(void)
{
	allocations = g_array_new(FALSE, FALSE, sizeof(uiprivMem));
}

void uiprivUninitAlloc(void)
{
	if (allocations->len == 0) {
		g_array_free(allocations, TRUE);
		return;
	}

	char *str = g_strdup("");
	for (guint i = 0; i < allocations->len; ++i) {
		uiprivMem mem = g_array_index(allocations, uiprivMem, i);
		char *new_str = g_strdup_printf("%s%p %s\n", str, mem.p, mem.type);
		g_free(str);
		str = new_str;
	}
	uiprivUserBug("Some data was leaked; either you left a uiControl lying around or there's a bug in libui itself. Leaked data:\n%s", str);
	g_free(str);
}

void *uiprivAlloc(size_t size, const char *type)
{
	return uiprivRealloc(NULL, size, type);
}

void *uiprivRealloc(void *p, size_t size, const char *type)
{
	uiprivMem mem = {.type = type, .p = p};
	if (p != NULL) {
		if (!removeAllocEntry(mem)) {
			uiprivImplBug("called uiprivRealloc() on unknown ptr %p", p);
		}
	} else if (size == 0) {
		uiprivImplBug("attempt to free NULL");
	}
	mem.p = g_realloc(p, size);
	if (mem.p != NULL) {
		memset(mem.p, 0, size);
		addAllocEntry(mem);
	}
	return mem.p;
}

void uiprivFree(void *p)
{
	uiprivRealloc(p, 0, NULL);
}
