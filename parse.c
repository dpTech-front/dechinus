/*
 *  dechinus wm (form of echinus) - maintainance: Daniel B. Prodigalidad
 *  this file contains code to parse rules and keybindings
 */
#define _GNU_SOURCE
#include <regex.h>
#include <ctype.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include "dechinus.h"
#include "config.h"

typedef struct {
	const char *name;
	void (*action) (const char *arg);
} KeyItem;

static KeyItem KeyItems[] = {
	{ "togglestruts",	togglestruts	},
	{ "focusicon",		focusicon	},
	{ "focusnext",		focusnext	},
	{ "focusprev",		focusprev	},
	{ "viewprevtag",	viewprevtag	},
	{ "viewlefttag",	viewlefttag	},
	{ "viewrighttag",	viewrighttag	},
	{ "quit",		quit		},
	{ "restart", 		quit		},
	{ "killclient",		killclient	},
	{ "togglefloating", 	togglefloating	},
	{ "decmwfact", 		setmwfact	},
	{ "incmwfact", 		setmwfact	},
	{ "incnmaster", 	incnmaster	},
	{ "decnmaster", 	incnmaster	},
	{ "iconify", 		iconify		},
	{ "zoom", 		zoom		},
	{ "moveright", 		moveresizekb	},
	{ "moveleft", 		moveresizekb	},
	{ "moveup", 		moveresizekb	},
	{ "movedown", 		moveresizekb	},
	{ "resizedecx", 	moveresizekb	},
	{ "resizeincx", 	moveresizekb	},
	{ "resizedecy", 	moveresizekb	},
	{ "resizeincy", 	moveresizekb	},
	{ "togglemonitor", 	togglemonitor	},
	{ "togglefill", 	togglefill	},
};

static KeyItem KeyItemsByTag[] = {
	{ "view",		view		},
	{ "toggleview",		toggleview	},
	{ "focusview",		focusview	},
	{ "tag", 		tag		},
	{ "toggletag", 		toggletag	},
};

static void
parsekey(const char *s, Key *k) {
	int l = strlen(s);
	unsigned long modmask = 0;
	char *pos, *opos;
	const char *stmp;
	char *tmp;
	int i;

	pos = strchr(s, '+');
	if ((pos - s) && pos) {
		for (i = 0, stmp = s; stmp < pos; i++, stmp++) {
			switch(*stmp) {
			case 'A':
				modmask = modmask | Mod1Mask;
				break;
			case 'S':
				modmask = modmask | ShiftMask;
				break;
			case 'C':
				modmask = modmask | ControlMask;
				break;
			case 'W':
				modmask = modmask | Mod4Mask;
				break;
			}
		}
	} else
		pos = (char *) s;
	opos = pos;
	k->mod = modmask;
	pos = strchr(s, '=');
	if (pos) {
		if (pos - opos < 0)
			opos = (char *) s;
		tmp = emallocz(pos - opos);
		for (; !isalnum(opos[0]); opos++);
		strncpy(tmp, opos, pos - opos - 1);
		k->keysym = XStringToKeysym(tmp);
		free(tmp);
		tmp = emallocz((s + l - pos + 1));
		for (pos++; !isgraph(pos[0]); pos++);
		strncpy(tmp, pos, s + l - pos);
		k->arg = tmp;
	} else {
		tmp = emallocz((s + l - opos));
		for (opos++; !isalnum(opos[0]); opos++);
		strncpy(tmp, opos, s + l - opos);
		k->keysym = XStringToKeysym(tmp);
		free(tmp);
	}
}

static void
initmodkey() {
	char tmp;

	strncpy(&tmp, getresource("modkey", "A"), 1);
	switch (tmp) {
	case 'S':
		modkey = ShiftMask;
		break;
	case 'C':
		modkey = ControlMask;
		break;
	case 'W':
		modkey = Mod4Mask;
		break;
	default:
		modkey = Mod1Mask;
	}
}

int
initkeys() {
	unsigned int i, j;
	const char *tmp;
	char t[64];

	initmodkey();
	keys = malloc(sizeof(Key *) * LENGTH(KeyItems));
	/* global functions */
	for (i = 0; i < LENGTH(KeyItems); i++) {
		tmp = getresource(KeyItems[i].name, NULL);
		if (!tmp)
			continue;
		keys[nkeys] = malloc(sizeof(Key));
		keys[nkeys]->func = KeyItems[i].action;
		keys[nkeys]->arg = NULL;
		parsekey(tmp, keys[nkeys]);
		nkeys++;
	}
	/* per tag functions */
	for (j = 0; j < LENGTH(KeyItemsByTag); j++) {
		for (i = 0; i < ntags; i++) {
			snprintf(t, sizeof(t), "%s%d", KeyItemsByTag[j].name, i);
			tmp = getresource(t, NULL);
			if (!tmp)
				continue;
			keys = realloc(keys, sizeof(Key *) * (nkeys + 1));
			keys[nkeys] = malloc(sizeof(Key));
			keys[nkeys]->func = KeyItemsByTag[j].action;
			keys[nkeys]->arg = tags[i];
			parsekey(tmp, keys[nkeys]);
			nkeys++;
		}
	}
	/* layout setting */
	for (i = 0; layouts[i].symbol != '\0'; i++) {
		snprintf(t, sizeof(t), "setlayout%c", layouts[i].symbol);
		tmp = getresource(t, NULL);
		if (!tmp)
			continue;
		keys = realloc(keys, sizeof(Key *) * (nkeys + 1));
		keys[nkeys] = malloc(sizeof(Key));
		keys[nkeys]->func = setlayout;
		keys[nkeys]->arg = &layouts[i].symbol;
		parsekey(tmp, keys[nkeys]);
		nkeys++;
	}
	/* spawn */
	for (i = 0; i < 64; i++) {
		snprintf(t, sizeof(t), "spawn%d", i);
		tmp = getresource(t, NULL);
		if (!tmp)
			continue;
		keys = realloc(keys, sizeof(Key *) * (nkeys + 1));
		keys[nkeys] = malloc(sizeof(Key));
		keys[nkeys]->func = spawn;
		keys[nkeys]->arg = NULL;
		parsekey(tmp, keys[nkeys]);
		nkeys++;
	}
	return 0;
}

static void
parserule(const char *s, Rule *r) {
	r->prop = emallocz(128);
	r->tags = emallocz(64);
	sscanf(s, "%s %s %d %d", r->prop, r->tags, &r->isfloating, &r->hastitle);
}

static void
compileregs(void) {
	unsigned int i;
	regex_t *reg;

	for (i = 0; i < nrules; i++) {
		if (rules[i]->prop) {
			reg = emallocz(sizeof(regex_t));
			if (regcomp(reg, rules[i]->prop, REG_EXTENDED))
				free(reg);
			else
				rules[i]->propregex = reg;
		}
		if (rules[i]->tags) {
			reg = emallocz(sizeof(regex_t));
			if (regcomp(reg, rules[i]->tags, REG_EXTENDED))
				free(reg);
			else
				rules[i]->tagregex = reg;
		}
	}
}

void
initrules() {
	int i;
	char t[64];
	const char *tmp;
	rules = emallocz(64 * sizeof(Rule *));
	for (i = 0; i < 64; i++) {
		snprintf(t, sizeof(t), "rule%d", i);
		tmp = getresource(t, NULL);
		if (!tmp)
			continue;
		rules[nrules] = emallocz(sizeof(Rule));
		parserule(tmp, rules[nrules]);
		nrules++;
	}
	rules = realloc(rules, nrules * sizeof(Rule *));
	compileregs();
}
void
loadconfig(const char *file) {
	FILE *f; char buf[1024], *p, *key, *val; ConfigEntry *e;
	if (!(f = fopen(file, "r"))) return;
	while (fgets(buf, sizeof(buf), f)) {
		for (p = buf; isspace(*p); p++);
		if (*p == '#' || *p == '\0') continue;
		if (!(p = strchr(buf, ':'))) continue;
		*p = '\0'; val = p + 1; key = buf;
		if (strncmp(key, RESCLASS, strlen(RESCLASS)) == 0) key += strlen(RESCLASS) + 1;
		else if (strncmp(key, RESNAME, strlen(RESNAME)) == 0) key += strlen(RESNAME) + 1;
		while (isspace(*val)) val++;
		if ((p = strchr(val, '\n'))) *p = '\0';
		e = emallocz(sizeof(ConfigEntry));
		e->key = strdup(key); e->val = strdup(val); e->next = config; config = e;
	} 
	fclose(f);
}
