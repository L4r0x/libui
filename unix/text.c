#include "uipriv_unix.h"

char *uiUnixStrdupText(const char *t)
{
	return g_strdup(t);
}

void uiFreeText(char *t)
{
	g_free(t);
}

int uiprivStricmp(const char *a, const char *b)
{
	gchar *a_utf8 = g_utf8_casefold(a, strlen(a));
	gchar *b_utf8 = g_utf8_casefold(b, strlen(b));
	int result = g_strcmp0(a_utf8, b_utf8);
	g_free(a_utf8);
	g_free(b_utf8);
	return result;
}
