// 4 december 2014
#include "uipriv_windows.hpp"

static std::map<void *, const char *> types;

void initAlloc(void)
{
	// do nothing
}

void uninitAlloc(void)
{
	if (types.size() == 0)
		return;

	std::ostringstream oss;
	for (const auto &alloc : types) {
		oss << (void *)(alloc.first) << " " << alloc.second << "\n";
	}

	std::string ossstr = oss.str();
	uiprivUserBug("Memory leak detected; either you left a uiControl lying "
				  "around or there's a bug in libui itself. Leaked data:\n%s",
		ossstr.c_str());
}

void *uiprivAlloc(size_t size, const char *type)
{
	return uiprivRealloc(nullptr, size, type);
}

// Reallocates memory returning the new memory address.
// The memory is initialized with zero.
//
// If p is NULL new memory is allocated similar to malloc.
// If the size is 0 the memory is deallocated similar to free.
void *uiprivRealloc(void *p, size_t size, const char *type)
{
	if (p != nullptr) {
		types.erase(p);
	} else if (size == 0) {
		uiprivImplBug("attempt to free NULL");
	}
	void *out = realloc(p, size);
	if (out != nullptr) {
		memset(out, 0, size);
		types[out] = type;
	}
	return out;
}

void uiprivFree(void *p)
{
	uiprivRealloc(p, 0, nullptr);
}
