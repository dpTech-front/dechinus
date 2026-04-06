/* See LICENSE file for copyright and license details.
 *
 * echinus window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance.  Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of echinus are organized in an
 * array which is accessed whenever a new event has been fetched. This allows
 * event dispatching in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag.  Clients are organized in a global
 * doubly-linked client list, the focus history is remembered through a global
 * stack list. Each client contains an array of Bools of the same size as the
 * global tags array to indicate the tags of a client.	
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <regex.h>
#include <signal.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xft/Xft.h>
#ifdef XRANDR
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randr.h>
#endif
#include "dechinus.h"

/* macros */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)		(mask & ~(numlockmask | LockMask))
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)
#define CLIENTMASK	        (PropertyChangeMask | StructureNotifyMask | FocusChangeMask)
#define CLIENTNOPROPAGATEMASK 	(BUTTONMASK | ButtonMotionMask)
#define FRAMEMASK               (MOUSEMASK | SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask)

/* enums */
enum { StrutsOn, StrutsOff, StrutsHide };		    /* struts position */
enum { CurNormal, CurResize, CurMove, CurLast };	    /* cursor */
enum { Clk2Focus, SloppyFloat, AllSloppy, SloppyRaise };    /* focus model */

/* function declarations */
void applyatoms(Client * c);
void applyrules(Client * c);
void arrange(Monitor * m);
void attach(Client * c, Bool attachaside);
void attachstack(Client * c);
void ban(Client * c);
void buttonpress(XEvent * e);
void bstack(Monitor * m);
void checkotherwm(void);
void cleanup(void);
void compileregs(void);
void configure(Client * c);
void configurenotify(XEvent * e);
void configurerequest(XEvent * e);
void destroynotify(XEvent * e);
void detach(Client * c);
void detachstack(Client * c);
void *emallocz(unsigned int size);
void enternotify(XEvent * e);
void eprint(const char *errstr, ...);
void expose(XEvent * e);
void iconify(const char *arg);
void incnmaster(const char *arg);
void focus(Client * c);
void focusnext(const char *arg);
void focusprev(const char *arg);
Client *getclient(Window w, Client * list, int part);
const char *getresource(const char *resource, const char *defval);
long getstate(Window w);
Bool gettextprop(Window w, Atom atom, char *text, unsigned int size);
void getpointer(int *x, int *y);
Monitor *getmonitor(int x, int y);
Monitor *curmonitor();
Monitor *clientmonitor(Client * c);
int idxoftag(const char *tag);
Bool isvisible(Client * c, Monitor * m);
void initmonitors(XEvent * e);
void keypress(XEvent * e);
void killclient(const char *arg);
void leavenotify(XEvent * e);
void focusin(XEvent * e);
void manage(Window w, XWindowAttributes * wa);
void mappingnotify(XEvent * e);
void monocle(Monitor * m);
void maprequest(XEvent * e);
void mousemove(Client * c);
void mouseresize(Client * c);
void moveresizekb(const char *arg);
Client *nexttiled(Client * c, Monitor * m);
Client *prevtiled(Client * c, Monitor * m);
void place(Client *c);
void propertynotify(XEvent * e);
void reparentnotify(XEvent * e);
void quit(const char *arg);
void restart(const char *arg);
void resize(Client * c, int x, int y, int w, int h, Bool sizehints);
void restack(Monitor * m);
void run(void);
void save(Client * c);
void scan(void);
void setclientstate(Client * c, long state);
void setlayout(const char *arg);
void setmwfact(const char *arg);
void setup(char *);
void spawn(const char *arg);
void tag(const char *arg);
void tile(Monitor * m);
void togglestruts(const char *arg);
void togglefloating(const char *arg);
void togglemax(const char *arg);
void togglefill(const char *arg);
void toggletag(const char *arg);
void toggleview(const char *arg);
void togglemonitor(const char *arg);
void focusview(const char *arg);
void unban(Client * c);
void unmanage(Client * c);
void updategeom(Monitor * m);
void updatestruts(Monitor * m);
void unmapnotify(XEvent * e);
void updatesizehints(Client * c);
void updateframe(Client * c);
void updatetitle(Client * c);
void view(const char *arg);
void viewprevtag(const char *arg);	/* views previous selected tags */
void viewlefttag(const char *arg);
void viewrighttag(const char *arg);
int xerror(Display * dpy, XErrorEvent * ee);
int xerrordummy(Display * dsply, XErrorEvent * ee);
int xerrorstart(Display * dsply, XErrorEvent * ee);
int (*xerrorxlib) (Display *, XErrorEvent *);
void zoom(const char *arg);

/* variables */
char **cargv;
Display *dpy;
int screen;
Window root;
XrmDatabase xrdb;
Bool otherwm;
Bool running = True;
Bool selscreen = True;
Monitor *monitors;
Client *clients;
Client *sel;
Client *stack;
Cursor cursor[CurLast];
Style style;
Button button[LastBtn];
View *views;
Key **keys;
Rule **rules;
char **tags;
unsigned int ntags;
unsigned int nkeys;
unsigned int nrules;
unsigned int modkey;
unsigned int numlockmask;
/* configuration, allows nested code to access above variables */
#include "config.h"

struct {
	Bool attachaside;
	Bool dectiled;
	Bool hidebastards;
	int focus;
	int gap;
	int snap;
	char command[255];
} options;

Layout layouts[] = {
	/* function	symbol	features */
	{  NULL,	'i',	OVERLAP },
	{  tile,	't',	MWFACT | NMASTER | ZOOM },
	{  bstack,	'b',	MWFACT | ZOOM },
	{  monocle,	'm',	0 },
	{  NULL,	'f',	OVERLAP },
	{  NULL,	'\0',	0 },
};

void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ButtonRelease] = buttonpress,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[LeaveNotify] = leavenotify,
	[FocusIn] = focusin,
	[Expose] = expose,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[PropertyNotify] = propertynotify,
	[ReparentNotify] = reparentnotify,
	[UnmapNotify] = unmapnotify,
	[ClientMessage] = clientmessage,
#ifdef XRANDR
	[RRScreenChangeNotify] = initmonitors,
#endif
};

/* function implementations */
void
applyatoms(Client * c) {
	unsigned int *t;
	unsigned long n;
	int i;

	/* restore tag number from atom */
	t = (unsigned int*)getatom(c->win, atom[WindowDesk], &n);
	if (n != 0) {
		if (*t >= ntags)
			return;
		for (i = 0; i < ntags; i++)
			c->tags[i] = (i == *t) ? 1 : 0;
	}
}

void
applyrules(Client * c) {
	static char buf[512];
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch = { 0 };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	snprintf(buf, sizeof(buf), "%s:%s:%s",
	    ch.res_class ? ch.res_class : "", ch.res_name ? ch.res_name : "", c->name);
	buf[LENGTH(buf)-1] = 0;
	for (i = 0; i < nrules; i++)
		if (rules[i]->propregex && !regexec(rules[i]->propregex, buf, 1, &tmp, 0)) {
			c->isfloating = rules[i]->isfloating;
			c->title = rules[i]->hastitle;
			for (j = 0; rules[i]->tagregex && j < ntags; j++) {
				if (!regexec(rules[i]->tagregex, tags[j], 1, &tmp, 0)) {
					matched = True;
					c->tags[j] = True;
				}
			}
		}
	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
	if (!matched) {
		Monitor *m = curmonitor();
		if (!m) m = monitors; // Fallback to first monitor
		if (m) memcpy(c->tags, m->seltags, ntags * sizeof(m->seltags[0]));
	}
}

void
arrangefloats(Monitor * m) {
	Client *c;
	Monitor *om;
	int dx, dy;

	for (c = stack; c; c = c->snext) {
		if (isvisible(c, m) && !c->isbastard &&
			       	(c->isfloating || FEATURES(views[m->curtag].layout, OVERLAP))
			       	&& !c->ismax && !c->isicon) {
			DPRINTF("%d %d\n", c->rx, c->ry);
			if (!(om = getmonitor(c->rx + c->rw/2,
				       	c->ry + c->rh/2)))
				continue;
			dx = om->sx + om->sw - c->rx;
			dy = om->sy + om->sh - c->ry;
			if (dx > m->sw) 
				dx = m->sw;
			if (dy > m->sh) 
				dy = m->sh;
			resize(c, m->sx + m->sw - dx, m->sy + m->sh - dy, c->rw, c->rh, True);
			save(c);
		}
	}
}

void
arrangemon(Monitor * m) {
	Client *c;

	if (views[m->curtag].layout->arrange)
		views[m->curtag].layout->arrange(m);
	arrangefloats(m);
	restack(m);
	for (c = stack; c; c = c->snext) {
		if ((clientmonitor(c) == m) && ((!c->isbastard && !c->isicon) ||
			(c->isbastard && views[m->curtag].barpos == StrutsOn))) {
			unban(c);
		}
	}

	for (c = stack; c; c = c->snext) {
		if ((clientmonitor(c) == NULL) || (!c->isbastard && c->isicon) ||
			(c->isbastard && views[m->curtag].barpos == StrutsHide)) {
			ban(c);
		}
	}
}

void
arrange(Monitor * m) {
	Monitor *i;

	if (!m) {
		for (i = monitors; i; i = i->next)
			arrangemon(i);
	} else
		arrangemon(m);
}

void
attach(Client * c, Bool attachaside) {
	if (attachaside) {
		if (clients) {
			Client * lastClient = clients;
			while (lastClient->next)
				lastClient = lastClient->next;
			c->prev = lastClient;
			lastClient->next = c;
		}
		else
			clients = c;
	}
	else {
		if (clients)
			clients->prev = c;
		c->next = clients;
		clients = c;
	}
}

void
attachstack(Client * c) {
	c->snext = stack;
	stack = c;
}

void
ban(Client * c) {
	if (c->isbanned)
		return;
	c->ignoreunmap++;
	setclientstate(c, IconicState);
	XSelectInput(dpy, c->win, CLIENTMASK & ~(StructureNotifyMask | EnterWindowMask));
	XSelectInput(dpy, c->frame, NoEventMask);
	XUnmapWindow(dpy, c->frame);
	XUnmapWindow(dpy, c->win);
	XSelectInput(dpy, c->win, CLIENTMASK);
	XSelectInput(dpy, c->frame, FRAMEMASK);
	c->isbanned = True;
}

void
buttonpress(XEvent * e) {
	Client *c;
	int i;
	XButtonPressedEvent *ev = &e->xbutton;
	Monitor *m = curmonitor();

	if (!m)
		return;
	if (ev->window == root) {
		if (ev->type != ButtonRelease)
			return;
		switch (ev->button) {
		case Button3:
			spawn(options.command);
			break;
		case Button4:
			viewlefttag(NULL);
			break;
		case Button5:
			viewrighttag(NULL);
			break;
		}
		return;
	}
	if ((c = getclient(ev->window, clients, ClientTitle))) {
		DPRINTF("TITLE %s: 0x%x\n", c->name, (int) ev->window);
		focus(c);
		for (i = 0; i < LastBtn; i++) {
			if (button[i].action == NULL)
				continue;
			if ((ev->x > button[i].x)
			    && ((int)ev->x < (int)(button[i].x + style.titleheight))
			    && (button[i].x != -1) && (int)ev->y < style.titleheight) {
				if (ev->type == ButtonPress) {
					DPRINTF("BUTTON %d PRESSED\n", i);
					button[i].pressed = 1;
				} else {
					DPRINTF("BUTTON %d RELEASED\n", i);
					button[i].pressed = 0;
					button[i].action(NULL);
				}
				drawclient(c);
				return;
			}
		}
		for (i = 0; i < LastBtn; i++)
			button[i].pressed = 0;
		drawclient(c);
		if (ev->type == ButtonRelease)
			return;
		restack(m);
		if (ev->button == Button1)
			mousemove(c);
		else if (ev->button == Button3)
			mouseresize(c);
	} else if ((c = getclient(ev->window, clients, ClientWindow))) {
		DPRINTF("WINDOW %s: 0x%x\n", c->name, (int) ev->window);
		focus(c);
		restack(m);
		if (CLEANMASK(ev->state) != modkey) {
			XAllowEvents(dpy, ReplayPointer, CurrentTime);
			return;
		}
		if (ev->button == Button1) {
			if (!FEATURES(views[m->curtag].layout, OVERLAP) && !c->isfloating)
				togglefloating(NULL);
			if (c->ismax)
				togglemax(NULL);
			mousemove(c);
		} else if (ev->button == Button2) {
			if (!FEATURES(views[m->curtag].layout, OVERLAP) && c->isfloating)
				togglefloating(NULL);
			else
				zoom(NULL);
		} else if (ev->button == Button3) {
			if (!FEATURES(views[m->curtag].layout, OVERLAP) && !c->isfloating)
				togglefloating(NULL);
			if (c->ismax)
				togglemax(NULL);
			mouseresize(c);
		}
	}
}

void
checkotherwm(void) {
	otherwm = False;
	XSetErrorHandler(xerrorstart);

	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, root, SubstructureRedirectMask);
	XSync(dpy, False);
	if (otherwm)
		eprint("dechinus: another window manager is already running\n");
	XSync(dpy, False);
	XSetErrorHandler(NULL);
	xerrorxlib = XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void) {
	while (stack) {
		unban(stack);
		unmanage(stack);
	}
	free(tags);
	free(keys);
	initmonitors(NULL);
	/* free resource database */
	XrmDestroyDatabase(xrdb);
	deinitstyle();
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	XFreeCursor(dpy, cursor[CurNormal]);
	XFreeCursor(dpy, cursor[CurResize]);
	XFreeCursor(dpy, cursor[CurMove]);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XSync(dpy, False);
}

void
configure(Client * c) {
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h - c->th;
	ce.border_width = 0;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *) & ce);
}

void
configurenotify(XEvent * e) {
	XConfigureEvent *ev = &e->xconfigure;
	Monitor *m;
	Client *c;

	if (ev->window == root) {
#ifdef XRANDR
		if (XRRUpdateConfiguration((XEvent *) ev)) {
#endif
			initmonitors(e);
			for (c = clients; c; c = c->next) {
				if (c->isbastard) {
					m = getmonitor(c->x + c->w/2, c->y);
					c->tags = m->seltags;
					updatestruts(m);
				}
			}
			for (m = monitors; m; m = m->next)
				updategeom(m);
			arrange(NULL);
#ifdef XRANDR
		}
#endif
	}
}

void
configurerequest(XEvent * e) {
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	Monitor *cm;
	int x, y, w, h;

	if ((c = getclient(ev->window, clients, ClientWindow))) {
		cm = clientmonitor(c);
		if (ev->value_mask & CWBorderWidth)
			c->border = ev->border_width;
		if (c->isfixed || c->isfloating || (cm && FEATURES(views[cm->curtag].layout, OVERLAP))) {
			if (ev->value_mask & CWX)
				x = ev->x;
			if (ev->value_mask & CWY)
				y = ev->y;
			if (ev->value_mask & CWWidth)
				w = ev->width;
			if (ev->value_mask & CWHeight)
				h = ev->height + c->th;
			cm = getmonitor(x, y);
			if (!(ev->value_mask & (CWX | CWY)) /* resize request */
			    && (ev->value_mask & (CWWidth | CWHeight))) {
				resize(c, c->x, c->y, w, h, True);
				save(c);
			} else if ((ev->value_mask & (CWX | CWY)) /* move request */
			    && !(ev->value_mask & (CWWidth | CWHeight))) {
				resize(c, x, y, c->w, c->h, True);
				save(c);
			} else if ((ev->value_mask & (CWX | CWY)) /* move and resize request */
			    && (ev->value_mask & (CWWidth | CWHeight))) {
				resize(c, x, y, w, h, True);
				save(c);
			} else if ((ev->value_mask & CWStackMode)) {
				configure(c);
			}
		} else {
			configure(c);
		}
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

void
destroynotify(XEvent * e) {
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if (!(c = getclient(ev->window, clients, ClientWindow)))
		return;
	unmanage(c);
	updateatom[ClientList] (NULL);
}

void
detach(Client * c) {
	if (c->prev)
		c->prev->next = c->next;
	if (c->next)
		c->next->prev = c->prev;
	if (c == clients)
		clients = c->next;
	c->next = c->prev = NULL;
}

void
detachstack(Client * c) {
	Client **tc;

	for (tc = &stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;
}

void *
emallocz(unsigned int size) {
	void *res = calloc(1, size);

	if (!res)
		eprint("fatal: could not malloc() %u bytes\n", size);
	return res;
}

void
enternotify(XEvent * e) {
	XCrossingEvent *ev = &e->xcrossing;
	Client *c;
	Monitor *m = curmonitor();

	if (ev->mode != NotifyNormal || ev->detail == NotifyInferior || !m)
		return;
	if ((c = getclient(ev->window, clients, ClientFrame))) {
		if (c->isbastard)
			return;
		if (!isvisible(sel, m))
			focus(c);
		switch (options.focus) {
		case Clk2Focus:
			break;
		case SloppyFloat:
			if (FEATURES(views[m->curtag].layout, OVERLAP) || c->isfloating)
				focus(c);
			break;
		case AllSloppy:
			focus(c);
			break;
		case SloppyRaise:
			focus(c);
			restack(m);
			break;
		}
	} else if (ev->window == root) {
		selscreen = True;
		focus(NULL);
	}
}

void
eprint(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void
focusin(XEvent * e) {
	XFocusChangeEvent *ev = &e->xfocus;
	Client *c;

	if (sel && ((c = getclient(ev->window, clients, ClientWindow)) != sel))
		XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
}

void
expose(XEvent * e) {
	XExposeEvent *ev = &e->xexpose;
	XEvent tmp;
	Client *c;

	while (XCheckWindowEvent(dpy, ev->window, ExposureMask, &tmp));
	if ((c = getclient(ev->window, clients, ClientTitle)))
		drawclient(c);
}

void
givefocus(Client * c) {
	XEvent ce;
	if (checkatom(c->win, atom[WMProto], atom[WMTakeFocus])) {
		ce.xclient.type = ClientMessage;
		ce.xclient.message_type = atom[WMProto];
		ce.xclient.display = dpy;
		ce.xclient.window = c->win;
		ce.xclient.format = 32;
		ce.xclient.data.l[0] = atom[WMTakeFocus];
		ce.xclient.data.l[1] = CurrentTime;
		ce.xclient.data.l[2] = ce.xclient.data.l[3] = ce.xclient.data.l[4] = 0l;
		XSendEvent(dpy, c->win, False, NoEventMask, &ce);
	}
}

void
focus(Client * c) {
	Client *o;
	Monitor *m = curmonitor();
	if (!m) return;

	o = sel;
	if ((!c && selscreen) || (c && (c->isbastard || !isvisible(c, m))))
		for (c = stack;
		    c && (c->isbastard || c->isicon || !isvisible(c, m)); c = c->snext);
	if (sel && sel != c) {
		XSetWindowBorder(dpy, sel->frame, style.color.norm[ColBorder]);
	}
	if (c) {
		detachstack(c);
		attachstack(c);
	}
	sel = c;
	if (!selscreen)
		return;
	if (c) {
		setclientstate(c, NormalState);
		if (c->isfocusable) {
			XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
			givefocus(c);
		}
		XSetWindowBorder(dpy, sel->frame, style.color.sel[ColBorder]);
		drawclient(c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	}
	if (o)
		drawclient(o);
	updateatom[ActiveWindow] (sel);
	updateatom[ClientList] (NULL);
	updateatom[CurDesk] (NULL);
}

void
focusicon(const char *arg) {
	Client *c;
	Monitor *m = curmonitor();
	if (!m) return;

	for (c = clients; c && (!c->isicon || !isvisible(c, m)); c = c->next);
	if (!c)
		return;
	c->isicon = False;
	focus(c);
	arrange(m);
}

void
focusnext(const char *arg) {
	Client *c;
	Monitor *m = curmonitor();
	if (!sel || !m)
		return;
	for (c = sel->next;
	    c && (c->isbastard || c->isicon || !isvisible(c, m)); c = c->next);
	if (!c)
		for (c = clients;
		    c && (c->isbastard || c->isicon
			|| !isvisible(c, m)); c = c->next);
	if (c) {
		focus(c);
		restack(m);
	}
}

void
focusprev(const char *arg) {
	Client *c;
	Monitor *m = curmonitor();
	if (!sel || !m)
		return;
	for (c = sel->prev;
	    c && (c->isbastard || c->isicon || !isvisible(c, m)); c = c->prev);
	if (!c) {
		for (c = clients; c && c->next; c = c->next);
		for (;
		    c && (c->isbastard || c->isicon
			|| !isvisible(c, m)); c = c->prev);
	}
	if (c) {
		focus(c);
		restack(m);
	}
}

void
iconify(const char *arg) {
	Client *c;
	Monitor *m = curmonitor();
	if (!sel || !m)
		return;
	c = sel;
	focusnext(NULL);
	ban(c);
	c->isicon = True;
	arrange(m);
}

void
incnmaster(const char *arg) {
	unsigned int i;
	Monitor *m = curmonitor();
	if (!m) return;

	if (!FEATURES(views[m->curtag].layout, NMASTER))
		return;
	if (!arg) {
		views[m->curtag].nmaster = DEFNMASTER;
	} else {
		i = atoi(arg);
		if ((views[m->curtag].nmaster + (int)i) < 1
		    || m->wah / (views[m->curtag].nmaster + i) <= (int)(2 * style.border))
			return;
		views[m->curtag].nmaster += i;
	}
	if (sel)
		arrange(m);
}

Client *
getclient(Window w, Client * list, int part) {
	Client *c;
#define ClientPart(_c, _part) (((_part) == ClientWindow) ? (_c)->win : \
			       ((_part) == ClientTitle) ? (_c)->title : \
			       ((_part) == ClientFrame) ? (_c)->frame : 0)

	for (c = list; c && (ClientPart(c, part)) != w; c = c->next);
	return c;
}

long
getstate(Window w) {
	long ret = -1;
	long *p = NULL;
	unsigned long n;
	p = (long*)getatom(w, atom[WMState], &n);
	if (n != 0)
		ret = *p;
	XFree(p);
	return ret;
}

const char *
getresource(const char *resource, const char *defval) {
	static char name[256], class[256], *type;
	XrmValue value;
	snprintf(name, sizeof(name), "%s.%s", RESNAME, resource);
	snprintf(class, sizeof(class), "%s.%s", RESCLASS, resource);
	XrmGetResource(xrdb, name, class, &type, &value);
	if (value.addr)
		return value.addr;
	return defval;
}

Bool
gettextprop(Window w, Atom atom, char *text, unsigned int size) {
	char **list = NULL;
	int n;
	XTextProperty name;
	if (!text || size == 0)
		return False;
	text[0] = '\0';
	XGetTextProperty(dpy, w, &name, atom);
	if (!name.nitems)
		return False;
	if (name.encoding == XA_STRING) {
		strncpy(text, (char *) name.value, size - 1);
	} else {
		if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
		    && n > 0 && *list) {
			strncpy(text, *list, size - 1);
			XFreeStringList(list);
		}
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return True;
}

int
idxoftag(const char *tag) {
	unsigned int i;
	for (i = 0; (i < ntags) && strcmp(tag, tags[i]); i++);
	return (i < ntags) ? i : 0;
}

Bool
isvisible(Client * c, Monitor * m) {
	unsigned int i;
	if (!c)
		return False;
	if (!m) {
		for (m = monitors; m; m = m->next) {
			for (i = 0; i < ntags; i++)
				if (c->tags[i] && m->seltags[i])
					return True;
		}
	} else {
		for (i = 0; i < ntags; i++)
			if (c->tags[i] && m->seltags[i])
				return True;
	}
	return False;
}

void
grabkeys(void) {
	unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
	unsigned int i, j;
	KeyCode code;
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	for (i = 0; i < nkeys; i++) {
		if ((code = XKeysymToKeycode(dpy, keys[i]->keysym))) {
			for (j = 0; j < LENGTH(modifiers); j++)
				XGrabKey(dpy, code, keys[i]->mod | modifiers[j], root,
					 True, GrabModeAsync, GrabModeAsync);
		}
    }
}

void
keypress(XEvent * e) {
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;
	if (!curmonitor())
		return;
	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode) ev->keycode, 0);
	for (i = 0; i < nkeys; i++)
		if (keysym == keys[i]->keysym
		    && CLEANMASK(keys[i]->mod) == CLEANMASK(ev->state)) {
			if (keys[i]->func)
				keys[i]->func(keys[i]->arg);
			XUngrabKeyboard(dpy, CurrentTime);
		}
}

void
killclient(const char *arg) {
	XEvent ev;
	if (!sel)
		return;
	if (checkatom(sel->win, atom[WMProto], atom[WMDelete])) {
		ev.type = ClientMessage;
		ev.xclient.window = sel->win;
		ev.xclient.message_type = atom[WMProto];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = atom[WMDelete];
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, sel->win, False, NoEventMask, &ev);
	} else {
		XKillClient(dpy, sel->win);
	}
}

void
leavenotify(XEvent * e) {
	XCrossingEvent *ev = &e->xcrossing;
	if ((ev->window == root) && !ev->same_screen) {
		selscreen = False;
		focus(NULL);
	}
}

void
manage(Window w, XWindowAttributes * wa) {
	Client *c, *t = NULL;
	Monitor *cm = NULL;
	Window trans;
	XWindowChanges wc;
	XSetWindowAttributes twa;
	XWMHints *wmh;
	unsigned long mask = 0;

	c = emallocz(sizeof(Client));
	c->win = w;
	if (checkatom(c->win, atom[WindowType], atom[WindowTypeDesk]) ||
	    checkatom(c->win, atom[WindowType], atom[WindowTypeDock])) {
		c->isbastard = True;
		c->isfloating = True;
		c->isfixed = True;
	}
	if (checkatom(c->win, atom[WindowType], atom[WindowTypeDialog])) {
		c->isfloating = True;
		c->isfixed = True;
	}

	cm = curmonitor();
	c->isicon = False;
	c->title = c->isbastard ? (Window) NULL : 1;
	c->tags = emallocz(ntags * sizeof(cm->seltags[0]));
	c->isfocusable = c->isbastard ? False : True;
	c->border = c->isbastard ? 0 : style.border;
	c->oldborder = c->isbastard ? 0 : wa->border_width;
	c->ignoreunmap = wa->map_state == IsViewable ? 1 : 0;
	mwm_process_atom(c);
	updatesizehints(c);
	updatetitle(c);
	applyrules(c);
	applyatoms(c);

	if (XGetTransientForHint(dpy, w, &trans)) {
		if ((t = getclient(trans, clients, ClientWindow))) {
			memcpy(c->tags, t->tags, ntags * sizeof(cm->seltags[0]));
			c->isfloating = True;
		}
	}
	c->th = c->title ? style.titleheight : 0;
	if (!c->isfloating)
		c->isfloating = c->isfixed;
	if ((wmh = XGetWMHints(dpy, c->win))) {
		c->isfocusable = !(wmh->flags & InputHint) || wmh->input;
		XFree(wmh);
	}
	c->x = c->rx = wa->x;
	c->y = c->ry = wa->y;
	c->w = c->rw = wa->width;
	c->h = c->rh = wa->height + c->th;
	if (!wa->x && !wa->y && !c->isbastard)
		place(c);
	cm = c->isbastard ? getmonitor(wa->x, wa->y) : clientmonitor(c);
	if (!cm) cm = curmonitor();
	c->hasstruts = getstruts(c); 
	if (c->isbastard) {
		free(c->tags);
		c->tags = cm->seltags;
	}

	XGrabButton(dpy, AnyButton, AnyModifier, c->win, True,
			ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
	twa.override_redirect = True;
	twa.event_mask = FRAMEMASK;
	mask = CWOverrideRedirect | CWEventMask;
	if (wa->depth == 32) {
		mask |= CWColormap | CWBorderPixel | CWBackPixel;
		twa.colormap = XCreateColormap(dpy, root, wa->visual, AllocNone);
		twa.background_pixel = BlackPixel(dpy, screen);
		twa.border_pixel = BlackPixel(dpy, screen);
	}
	c->frame =
	    XCreateWindow(dpy, root, c->x, c->y, c->w,
	    c->h, c->border, wa->depth == 32 ? 32 : DefaultDepth(dpy, screen),
	    InputOutput, wa->depth == 32 ? wa->visual : DefaultVisual(dpy,
		screen), mask, &twa);

	wc.border_width = c->border;
	XConfigureWindow(dpy, c->frame, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, c->frame, style.color.norm[ColBorder]);

	twa.event_mask = ExposureMask | MOUSEMASK;
	if (c->title) {
		c->title = XCreateWindow(dpy, root, 0, 0, c->w, c->th,
		    0, DefaultDepth(dpy, screen), CopyFromParent,
		    DefaultVisual(dpy, screen), CWEventMask, &twa);
		c->drawable =
		    XCreatePixmap(dpy, root, c->w, c->th, DefaultDepth(dpy, screen));
		c->xftdraw =
		    XftDrawCreate(dpy, c->drawable, DefaultVisual(dpy, screen),
		    DefaultColormap(dpy, screen));
	}
	attach(c, options.attachaside);
	attachstack(c);
	twa.event_mask = CLIENTMASK;
	twa.do_not_propagate_mask = CLIENTNOPROPAGATEMASK;
	XChangeWindowAttributes(dpy, c->win, CWEventMask|CWDontPropagate, &twa);
	XReparentWindow(dpy, c->win, c->frame, 0, c->th);
	XReparentWindow(dpy, c->title, c->frame, 0, 0);
	XAddToSaveSet(dpy, c->win);
	XMapWindow(dpy, c->win);
	wc.border_width = 0;
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
	configure(c);
	if (checkatom(c->win, atom[WindowState], atom[WindowStateFs]))
		ewmh_process_state_atom(c, atom[WindowStateFs], 1);
	ban(c);
	updateatom[ClientList] (NULL);
	updateatom[WindowDesk] (c);
	updateframe(c);
	if (cm) {
		if (c->hasstruts) updategeom(cm);
		arrange(cm);
	}
	focus(NULL);
}

void
mappingnotify(XEvent * e) {
	XMappingEvent *ev = &e->xmapping;
	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent * e) {
	static XWindowAttributes wa;
	Client *c;
	XMapRequestEvent *ev = &e->xmaprequest;
	if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect)
		return;
	if (!(c = getclient(ev->window, clients, ClientWindow)))
		manage(ev->window, &wa);
}

void
monocle(Monitor * m) {
	Client *c;
	for (c = nexttiled(clients, m); c; c = nexttiled(c->next, m)) {
			if (views[m->curtag].barpos != StrutsOn)
				resize(c, m->wax - c->border,
						m->way - c->border, m->waw, m->wah, False);
			else
				resize(c, m->wax, m->way,
						m->waw - 2 * c->border,
						m->wah - 2 * c->border, False);
	}
}

void
moveresizekb(const char *arg) {
	int dw, dh, dx, dy;
	dw = dh = dx = dy = 0;
	if (!sel || !sel->isfloating)
		return;
	sscanf(arg, "%d %d %d %d", &dx, &dy, &dw, &dh);
	if (dw && (dw < sel->incw)) dw = (dw / abs(dw)) * sel->incw;
	if (dh && (dh < sel->inch)) dh = (dh / abs(dh)) * sel->inch;
	resize(sel, sel->x + dx, sel->y + dy, sel->w + dw,
	    sel->h + dh, True);
}

void
getpointer(int *x, int *y) {
	int di;
	unsigned int dui;
	Window dummy;
	XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

Monitor *
getmonitor(int x, int y) {
	Monitor *m;
	for (m = monitors; m; m = m->next) {
		if ((x >= m->sx && x <= m->sx + m->sw) &&
		    (y >= m->sy && y <= m->sy + m->sh))
			return m;
	}
	return NULL;
}

Monitor *
clientmonitor(Client * c) {
	Monitor *m;
	assert(c != NULL);
	for (m = monitors; m; m = m->next)
		if (isvisible(c, m))
			return m;
	return NULL;
}

Monitor *
curmonitor() {
	int x, y;
	getpointer(&x, &y);
	return getmonitor(x, y);
}

void
mousemove(Client * c) {
	int x1, y1, ocx, ocy, nx, ny;
	unsigned int i;
	XEvent ev;
	Monitor *m, *nm;

	if (c->isbastard)
		return;
	m = curmonitor();
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
		GrabModeAsync, None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	getpointer(&x1, &y1);
	for (;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		case MotionNotify:
			XSync(dpy, False);
			if (!(nm = curmonitor())) break;
			nx = ocx + (ev.xmotion.x_root - x1);
			ny = ocy + (ev.xmotion.y_root - y1);
			if (abs(nx - nm->wax) < options.snap) nx = nm->wax;
			else if (abs((nm->wax + nm->waw) - (nx + c->w + 2 * c->border)) < options.snap)
				nx = nm->wax + nm->waw - c->w - 2 * c->border;
			if (abs(ny - nm->way) < options.snap) ny = nm->way;
			else if (abs((nm->way + nm->wah) - (ny + c->h + 2 * c->border)) < options.snap)
				ny = nm->way + nm->wah - c->h - 2 * c->border;
			resize(c, nx, ny, c->w, c->h, True);
			save(c);
			if (m != nm) {
				for (i = 0; i < ntags; i++) c->tags[i] = nm->seltags[i];
				updateatom[WindowDesk] (c);
				drawclient(c);
				arrange(NULL);
				m = nm;
			}
			break;
		default:
			if (handler[ev.type]) handler[ev.type] (&ev);
			break;
		}
	}
}

void
mouseresize(Client * c) {
	int ocx, ocy, nw, nh;
	XEvent ev;
	if (c->isbastard || c->isfixed) return;
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
		GrabModeAsync, None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	c->ismax = False;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->border - 1, c->h + c->border - 1);
	for (;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ButtonRelease:
			XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->border - 1, c->h + c->border - 1);
			XUngrabPointer(dpy, CurrentTime);
			return;
		case MotionNotify:
			XSync(dpy, False);
			if ((nw = ev.xmotion.x - ocx - 2 * c->border + 1) <= 0) nw = 1;
			if ((nh = ev.xmotion.y - ocy - 2 * c->border + 1) <= 0) nh = 1;
			resize(c, c->x, c->y, nw, nh, True);
			save(c);
			break;
		default:
			if (handler[ev.type]) handler[ev.type] (&ev);
			break;
		}
	}
}

Client *
nexttiled(Client * c, Monitor * m) {
	for (; c && (c->isfloating || !isvisible(c, m) || c->isbastard || c->isicon); c = c->next);
	return c;
}

Client *
prevtiled(Client * c, Monitor * m) {
	for (; c && (c->isfloating || !isvisible(c, m) || c->isbastard || c->isicon); c = c->prev);
	return c;
}

void
reparentnotify(XEvent * e) {
	Client *c;
	XReparentEvent *ev = &e->xreparent;
	if ((c = getclient(ev->window, clients, ClientWindow)))
		if (ev->parent != c->frame) unmanage(c);
}

void
place(Client *c) {
	int x, y;
	Monitor *m;
	int d = style.titleheight;
	getpointer(&x, &y);
	m = getmonitor(x, y);
	if (!m) m = monitors;
	x = x + rand()%d - c->w/2;
	y = y + rand()%d - c->h/2;
	if (x < m->wax) x = m->wax;
	if (y < m->way) y = m->way;
	if (x + c->w > m->wax + m->waw) x = m->wax + m->waw - c->w - rand()%d;
	if (y + c->h > m->way + m->wah) y = m->way + m->wah - c->h - rand()%d;
	c->rx = c->x = x;
	c->ry = c->y = y;
}

void
propertynotify(XEvent * e) {
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;
	if ((c = getclient(ev->window, clients, ClientWindow))) {
		if (ev->atom == atom[StrutPartial]) {
			c->hasstruts = getstruts(c);
			updategeom(clientmonitor(c));
			arrange(clientmonitor(c));
		}
		if (ev->state == PropertyDelete) return;
		switch (ev->atom) {
		case XA_WM_TRANSIENT_FOR:
			XGetTransientForHint(dpy, c->win, &trans);
			if (!c->isfloating && (c->isfloating = (getclient(trans, clients, ClientWindow) != NULL)))
				arrange(clientmonitor(c));
			break;
		case XA_WM_NORMAL_HINTS: updatesizehints(c); break;
		case XA_WM_NAME:
		case 0: /* WindowName Atom */
			updatetitle(c);
			drawclient(c);
			break;
		}
	}
}

void
quit(const char *arg) {
	running = False;
	if (arg) {
		cleanup();
		execvp(cargv[0], cargv);
		eprint("dechinus: Can't exec: %s\n", strerror(errno));
	}
}

void
resize(Client * c, int x, int y, int w, int h, Bool sizehints) {
	if (sizehints) {
		h -= c->th;
		if (w < 1) w = 1;
		if (h < 1) h = 1;
		w -= c->basew; h -= c->baseh;
		if (c->minay > 0 && c->maxay > 0 && c->minax > 0 && c->maxax > 0) {
			if (w * c->maxay > h * c->maxax) w = h * c->maxax / c->maxay;
			else if (w * c->minay < h * c->minax) h = w * c->minay / c->minax;
		}
		if (c->incw) w -= w % c->incw;
		if (c->inch) h -= h % c->inch;
		w += c->basew; h += c->baseh;
		if (c->minw > 0 && w < c->minw) w = c->minw;
		if (c->minh > 0 && h - c->th < c->minh) h = c->minh + c->th;
		if (c->maxw > 0 && w > c->maxw) w = c->maxw;
		if (c->maxh > 0 && h - c->th > c->maxh) h = c->maxh + c->th;
		h += c->th;
	}
	if (w <= 0 || h <= 0) return;
	if (x > DisplayWidth(dpy, screen)) x = DisplayWidth(dpy, screen) - w - 2 * c->border;
	if (y > DisplayHeight(dpy, screen)) y = DisplayHeight(dpy, screen) - h - 2 * c->border;
	if (w != c->w && c->th) {
		XMoveResizeWindow(dpy, c->title, 0, 0, w, c->th);
		XFreePixmap(dpy, c->drawable);
		c->drawable = XCreatePixmap(dpy, root, w, c->th, DefaultDepth(dpy, screen));
		drawclient(c);
	}
	c->x = x; c->y = y; c->w = w; c->h = h;
	XMoveResizeWindow(dpy, c->frame, c->x, c->y, c->w, c->h);
	XMoveResizeWindow(dpy, c->win, 0, c->th, c->w, c->h - c->th);
	configure(c);
	XSync(dpy, False);
}

void
restack(Monitor * m) {
	Client *c;
	XEvent ev;
	Window *wl;
	int i, n;
	if (!sel || !m) return;
	for (n = 0, c = stack; c; c = c->snext) if (isvisible(c, m) && !c->isicon) n++;
	if (!n) return;
	wl = malloc(sizeof(Window) * n);
	i = 0;
	for (c = stack; c && i < n; c = c->snext)
		if (isvisible(c, m) && !c->isicon && !c->isbastard && c->isfloating) wl[i++] = c->frame;
	for (c = stack; c && i < n; c = c->snext)
		if (isvisible(c, m) && !c->isicon && c->isbastard && !checkatom(c->win, atom[WindowType], atom[WindowTypeDesk])) wl[i++] = c->frame;
	for (c = stack; c && i < n; c = c->snext) 
		if (isvisible(c, m) && !c->isicon && !c->isfloating && !c->isbastard) wl[i++] = c->frame;
	for (c = stack; c && i < n; c = c->snext)
		if (isvisible(c, m) && !c->isicon && c->isbastard && checkatom(c->win, atom[WindowType], atom[WindowTypeDesk])) wl[i++] = c->frame;
	XRestackWindows(dpy, wl, i);
	free(wl);
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void) {
	fd_set rd;
	int xfd;
	XEvent ev;
	XSync(dpy, False);
	xfd = ConnectionNumber(dpy);
	while (running) {
		FD_ZERO(&rd); FD_SET(xfd, &rd);
		if (select(xfd + 1, &rd, NULL, NULL, NULL) == -1) {
			if (errno == EINTR) continue;
			eprint("dechinus: select failed\n");
		}
		while (XPending(dpy)) {
			XNextEvent(dpy, &ev);
			if (handler[ev.type]) (handler[ev.type]) (&ev);
		}
	}
}

void
save(Client *c) {
	c->rx = c->x; c->ry = c->y; c->rw = c->w; c->rh = c->h;
}

void
scan(void) {
	unsigned int i, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;
	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa) || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1)) continue;
			if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState || getstate(wins[i]) == NormalState) manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa)) continue;
			if (XGetTransientForHint(dpy, wins[i], &d1) && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState || getstate(wins[i]) == NormalState)) manage(wins[i], &wa);
		}
	}
	if (wins) XFree(wins);
}

void
setclientstate(Client * c, long state) {
	long data[] = { state, None };
	long winstate[2];
	XChangeProperty(dpy, c->win, atom[WMState], atom[WMState], 32, PropModeReplace, (unsigned char *) data, 2);
	if (state == NormalState) {
		c->isicon = False;
		XDeleteProperty(dpy, c->win, atom[WindowState]);
	} else {
		winstate[0] = atom[WindowStateHidden];
		XChangeProperty(dpy, c->win, atom[WindowState], XA_ATOM, 32, PropModeReplace, (unsigned char *) winstate, 1);
	}
}

void
setlayout(const char *arg) {
	unsigned int i;
	Client *c;
	Bool wasfloat;
	Monitor *m = curmonitor();
	if (!m) return;

	wasfloat = FEATURES(views[m->curtag].layout, OVERLAP);
	if (arg) {
		for (i = 0; i < LENGTH(layouts); i++) if (*arg == layouts[i].symbol) break;
		if (i != LENGTH(layouts)) views[m->curtag].layout = &layouts[i];
	}
	if (sel) {
		for (c = clients; c; c = c->next) {
			if (isvisible(c, m)) {
				if (wasfloat) save(c);
				if (wasfloat != FEATURES(views[m->curtag].layout, OVERLAP)) updateframe(c);
			}
		}
		arrange(m);
	}
	updateatom[ELayout] (NULL);
}

void
setmwfact(const char *arg) {
	double delta;
	Monitor *m = curmonitor();
	if (!m || !FEATURES(views[m->curtag].layout, MWFACT)) return;
	if (arg == NULL) views[m->curtag].mwfact = DEFMWFACT;
	else if (sscanf(arg, "%lf", &delta) == 1) {
		if (arg[0] == '+' || arg[0] == '-') views[m->curtag].mwfact += delta;
		else views[m->curtag].mwfact = delta;
		if (views[m->curtag].mwfact < 0.1) views[m->curtag].mwfact = 0.1;
		else if (views[m->curtag].mwfact > 0.9) views[m->curtag].mwfact = 0.9;
	}
	arrange(m);
}

void
initlayouts() {
	unsigned int i, j;
	char conf[32], ltname;
	float mwfact = atof(getresource("mwfact", STR(DEFMWFACT)));
	int nmaster = atoi(getresource("nmaster", STR(DEFNMASTER)));
	const char *deflayout = getresource("deflayout", "i");
	if (nmaster < 1) nmaster = 1;
	for (i = 0; i < ntags; i++) {
		views[i].layout = &layouts[0];
		snprintf(conf, sizeof(conf), "tags.layout%d", i);
		strncpy(&ltname, getresource(conf, deflayout), 1);
		for (j = 0; j < LENGTH(layouts); j++) if (layouts[j].symbol == ltname) { views[i].layout = &layouts[j]; break; }
		views[i].mwfact = mwfact;
		views[i].nmaster = nmaster;
		views[i].barpos = StrutsOn;
	}
	updateatom[ELayout] (NULL);
}

void
initmonitors(XEvent * e) {
	Monitor *m;
#ifdef XRANDR
	Monitor *t;
	XRRCrtcInfo *ci;
	XRRScreenResources *sr;
	int c, n, ncrtc = 0, dummy1, dummy2, major, minor;
	if (monitors) {
		m = monitors;
		do { t = m->next; free(m->seltags); free(m->prevtags); free(m); m = t; } while (m);
		monitors = NULL;
	}
	if (!running) return;
	if (XRRQueryExtension(dpy, &dummy1, &dummy2))
		if (XRRQueryVersion(dpy, &major, &minor) && major < 1) goto no_xrandr;
	sr = XRRGetScreenResources(dpy, root);
	if (!sr) goto no_xrandr;
	ncrtc = sr->ncrtc;
	for (c = 0, n = 0; c < ncrtc; c++) {
		ci = XRRGetCrtcInfo(dpy, sr, sr->crtcs[c]);
		if (!ci || ci->noutput == 0) { if(ci) XRRFreeCrtcInfo(ci); continue; }
		m = emallocz(sizeof(Monitor));
		m->sx = m->wax = ci->x; m->sy = m->way = ci->y;
		m->sw = m->waw = ci->width; m->sh = m->wah = ci->height;
		m->mx = m->sx + m->sw/2; m->my = m->sy + m->sh/2;
		m->curtag = n; m->prevtags = emallocz(ntags * sizeof(Bool));
		m->seltags = emallocz(ntags * sizeof(Bool)); m->seltags[n] = True;
		m->next = monitors; monitors = m; n++;
		XRRFreeCrtcInfo(ci);
	}
	XRRFreeScreenResources(sr);
	updateatom[WorkArea] (NULL);
	return;
no_xrandr:
#endif
	m = emallocz(sizeof(Monitor));
	m->sx = m->wax = 0; m->sy = m->way = 0;
	m->sw = m->waw = DisplayWidth(dpy, screen); m->sh = m->wah = DisplayHeight(dpy, screen);
	m->mx = m->sw/2; m->my = m->sh/2;
	m->curtag = 0; m->prevtags = emallocz(ntags * sizeof(Bool));
	m->seltags = emallocz(ntags * sizeof(Bool)); m->seltags[0] = True;
	monitors = m;
	updateatom[WorkArea] (NULL);
}

void
inittags() {
	unsigned int i; char tmp[25];
	ntags = atoi(getresource("tags.number", "5"));
	views = emallocz(ntags * sizeof(View));
	tags = emallocz(ntags * sizeof(char *));
	for (i = 0; i < ntags; i++) {
		tags[i] = emallocz(25);
		snprintf(tmp, sizeof(tmp), "tags.name%d", i);
		snprintf(tags[i], 25, "%s", getresource(tmp, "null"));
	}
}

void sighandler(int signum) { if (signum == SIGHUP) quit("HUP!"); else quit(NULL); }

void
setup(char *conf) {
	int d, i, j; unsigned int mask; Window w; Monitor *m;
	XModifierKeymap *modmap; XSetWindowAttributes wa;
	char oldcwd[256], path[256] = "/"; char *home, *slash;
	const char *confs[] = { conf, "%s/.config/dewm/dewmrc", SYSCONFPATH "/dewmrc", NULL };

	cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_bottom_right_corner);
	cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++) for (j = 0; j < modmap->max_keypermod; j++)
		if (modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock)) numlockmask = (1 << i);
	XFreeModifiermap(modmap);
	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask;
	wa.cursor = cursor[CurNormal];
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
	XrmInitialize();
	home = getenv("HOME"); if (!home) home = "/";
	if (!getcwd(oldcwd, sizeof(oldcwd))) eprint("dechinus: getcwd error\n");
	for (i = 0; confs[i] != NULL; i++) {
		if (*confs[i] == '\0') continue;
		snprintf(conf, 255, confs[i], home);
		slash = strrchr(conf, '/'); if (slash) { snprintf(path, slash - conf + 1, "%s", conf); chdir(path); }
		if ((xrdb = XrmGetFileDatabase(conf))) break;
	}
	if (!xrdb) fprintf(stderr, "dechinus: no config found\n");
	initewmh(); inittags(); initmonitors(NULL); initrules(); initkeys(); initlayouts();
	updateatom[NumberOfDesk] (NULL); updateatom[DeskNames] (NULL); updateatom[CurDesk] (NULL);
	grabkeys(); initstyle();
	options.attachaside = atoi(getresource("attachaside", "1"));
	strncpy(options.command, getresource("command", COMMAND), 254);
	options.dectiled = atoi(getresource("decoratetiled", STR(DECORATETILED)));
	options.hidebastards = atoi(getresource("hidebastards", "0"));
	options.focus = atoi(getresource("sloppy", "0"));
	options.gap = atoi(getresource("gap", STR(DEFGAP)));
	options.snap = atoi(getresource("snap", STR(SNAP)));
	for (m = monitors; m; m = m->next) { m->struts[0]=m->struts[1]=m->struts[2]=m->struts[3]=0; updategeom(m); }
	chdir(oldcwd);
	selscreen = XQueryPointer(dpy, root, &w, &w, &d, &d, &d, &d, &mask);
}

void
spawn(const char *arg) {
	if (!arg) return;
	if (fork() == 0) {
		if (fork() == 0) {
			if (dpy) close(ConnectionNumber(dpy));
			setsid(); execl("/bin/sh", "sh", "-c", arg, (char *)NULL);
		}
		exit(0);
	}
	wait(0);
}

void
tag(const char *arg) {
	unsigned int i; if (!sel) return;
	for (i = 0; i < ntags; i++) sel->tags[i] = (NULL == arg);
	sel->tags[idxoftag(arg)] = True;
	updateatom[WindowDesk] (sel); updateframe(sel); arrange(NULL); focus(NULL);
}

void
bstack(Monitor * m) {
	int i, n, nx, ny, nw, nh, mh, tw; Client *c, *mc;
	for (n = 0, c = nexttiled(clients, m); c; c = nexttiled(c->next, m)) n++;
	if (!n) return;
	mh = (n == 1) ? m->wah : views[m->curtag].mwfact * m->wah;
	tw = (n > 1) ? m->waw / (n - 1) : 0;
	nx = m->wax; ny = m->way;
	for (i = 0, mc = nexttiled(clients, m), c = mc; c; c = nexttiled(c->next, m), i++) {
		if (i == 0) { nh = mh - 2 * c->border; nw = m->waw - 2 * c->border; }
		else {
			if (i == 1) { nx = m->wax; ny += mc->h + 2 * mc->border; nh = (m->way + m->wah) - ny - 2 * c->border; }
			nw = (i + 1 == n) ? (m->wax + m->waw) - nx - 2 * c->border : tw - 2 * c->border;
		}
		resize(c, nx, ny, nw, nh, False);
		if (n > 1) nx += nw + 2 * c->border;
	}
}

void
tile(Monitor * m) {
	int nx, ny, nw, nh, mw, mh; unsigned int i, n, th; Client *c, *mc;
	for (n = 0, c = nexttiled(clients, m); c; c = nexttiled(c->next, m)) n++;
	if (!n) return;
	mw = (n <= views[m->curtag].nmaster) ? m->waw : views[m->curtag].mwfact * m->waw;
	mh = m->wah / (n <= views[m->curtag].nmaster ? n : views[m->curtag].nmaster);
	th = (n > views[m->curtag].nmaster) ? m->wah / (n - views[m->curtag].nmaster) : 0;
	nx = m->wax; ny = m->way;
	for (i = 0, mc = nexttiled(clients, m), c = mc; c; c = nexttiled(c->next, m), i++) {
		if (i < views[m->curtag].nmaster) {
			ny = m->way + i * mh; nw = mw - 2 * c->border; nh = mh - 2 * c->border;
			resize(c, nx + options.gap, ny + options.gap, nw - 2*options.gap, nh - 2*options.gap, False);
		} else {
			if (i == views[m->curtag].nmaster) { nx += mw; ny = m->way; }
			nw = m->wax + m->waw - nx - 2 * c->border; nh = th - 2 * c->border;
			resize(c, nx + options.gap, ny + options.gap, nw - 2*options.gap, nh - 2*options.gap, False);
			ny += th;
		}
	}
}

void
togglestruts(const char *arg) {
	Monitor *m = curmonitor(); if (!m) return;
	views[m->curtag].barpos = (views[m->curtag].barpos == StrutsOn) ? (options.hidebastards ? StrutsHide : StrutsOff) : StrutsOn;
	updategeom(m); arrange(m);
}

void
togglefloating(const char *arg) {
	Monitor *m = curmonitor(); if (!sel || !m || FEATURES(views[m->curtag].layout, OVERLAP)) return;
	sel->isfloating = !sel->isfloating; updateframe(sel);
	if (sel->isfloating) resize(sel, sel->rx, sel->ry, sel->rw, sel->rh, False);
	else save(sel);
	arrange(m);
}

void
togglefill(const char *arg) {
	Monitor *m = curmonitor(); Client *c; int x1, x2, y1, y2;
	if (!sel || !m || sel->isfixed || !sel->isfloating) return;
	x1 = m->wax; x2 = m->wax + m->waw; y1 = m->way; y2 = m->way + m->wah;
	for (c = clients; c; c = c->next) {
		if (isvisible(c, m) && c != sel && !c->isbastard && c->isfloating) {
			if (c->y + c->h > sel->y && c->y < sel->y + sel->h) {
				if (c->x < sel->x) x1 = max(x1, c->x + c->w); else x2 = min(x2, c->x);
			}
		}
	}
	if ((sel->isfill = !sel->isfill)) { save(sel); resize(sel, x1, y1, x2-x1-2*sel->border, y2-y1-2*sel->border, True); }
	else resize(sel, sel->rx, sel->ry, sel->rw, sel->rh, True);
}

void
togglemax(const char *arg) {
	Monitor *m = curmonitor(); if (!sel || !m || sel->isfixed) return;
	sel->ismax = !sel->ismax; updateframe(sel);
	if (sel->ismax) { save(sel); resize(sel, m->sx, m->sy, m->sw - 2*sel->border, m->sh - 2*sel->border, False); }
	else resize(sel, sel->rx, sel->ry, sel->rw, sel->rh, True);
}

void
toggletag(const char *arg) {
	unsigned int i, j; if (!sel) return;
	i = idxoftag(arg); sel->tags[i] = !sel->tags[i];
	for (j = 0; j < ntags && !sel->tags[j]; j++);
	if (j == ntags) sel->tags[i] = True;
	drawclient(sel); arrange(NULL);
}

void
togglemonitor(const char *arg) {
	Monitor *m, *cm; int x, y; getpointer(&x, &y);
	if (!(cm = getmonitor(x, y))) return;
	for (m = monitors; m && (m == cm); m = m->next);
	if (m) { XWarpPointer(dpy, None, root, 0, 0, 0, 0, m->sx + m->sw/2, m->sy + m->sh/2); focus(NULL); }
}

void
toggleview(const char *arg) {
	unsigned int i; Monitor *m, *cm = curmonitor(); if (!cm) return;
	i = idxoftag(arg);
	memcpy(cm->prevtags, cm->seltags, ntags * sizeof(Bool));
	cm->seltags[i] = !cm->seltags[i];
	for (i=0; i<ntags && !cm->seltags[i]; i++);
	if (i==ntags) cm->seltags[idxoftag(arg)] = True;
	arrange(cm); focus(NULL); updateatom[CurDesk] (NULL);
}

void
focusview(const char *arg) {
	Client *c; unsigned int i = idxoftag(arg);
	Monitor *m = curmonitor(); if (!m) return;
	toggleview(arg);
	if (!m->seltags[i]) return;
	for (c = stack; c; c = c->snext) if (c->tags[i] && !c->isbastard) { focus(c); break; }
}

void
unban(Client * c) {
	if (!c->isbanned) return;
	XMapWindow(dpy, c->win); XMapWindow(dpy, c->frame);
	setclientstate(c, NormalState); c->isbanned = False;
}

void
unmanage(Client * c) {
	Monitor *m = clientmonitor(c);
	XGrabServer(dpy);
	XSelectInput(dpy, c->frame, NoEventMask); XUnmapWindow(dpy, c->frame);
	if (c->title) { XftDrawDestroy(c->xftdraw); XFreePixmap(dpy, c->drawable); XDestroyWindow(dpy, c->title); }
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	XReparentWindow(dpy, c->win, root, c->x, c->y);
	detach(c); detachstack(c); if (sel == c) focus(NULL);
	setclientstate(c, WithdrawnState); XDestroyWindow(dpy, c->frame);
	if (!c->isbastard) free(c->tags); free(c);
	XSync(dpy, False); XUngrabServer(dpy);
	if (m) { updategeom(m); arrange(m); }
	updateatom[ClientList] (NULL);
}

void
updategeom(Monitor * m) {
	m->wax = m->sx; m->way = m->sy; m->waw = m->sw; m->wah = m->sh;
	if (views[m->curtag].barpos == StrutsOn) {
		m->wax += m->struts[LeftStrut]; m->way += m->struts[TopStrut];
		m->waw -= (m->struts[LeftStrut] + m->struts[RightStrut]);
		m->wah -= (m->struts[TopStrut] + m->struts[BotStrut]);
	}
	updateatom[WorkArea] (NULL);
}

void
updatestruts(Monitor *m) {
	Client *c; m->struts[0]=m->struts[1]=m->struts[2]=m->struts[3]=0;
	for (c = clients; c; c = c->next) if (c->hasstruts) getstruts(c);
}

void
unmapnotify(XEvent * e) {
	Client *c; XUnmapEvent *ev = &e->xunmap;
	if ((c = getclient(ev->window, clients, ClientWindow))) {
		if (c->ignoreunmap--) return;
		unmanage(c);
	}
}

void
updateframe(Client * c) {
	if (!c->title) return;
	c->th = !c->ismax ? style.titleheight : 0;
	if (!c->th) XUnmapWindow(dpy, c->title); else XMapRaised(dpy, c->title);
}

void
updatesizehints(Client * c) {
	XSizeHints size; long msize;
	if (!XGetWMNormalHints(dpy, c->win, &size, &msize)) size.flags = PSize;
	c->flags = size.flags;
	if (c->flags & PBaseSize) { c->basew = size.base_width; c->baseh = size.base_height; }
	else if (c->flags & PMinSize) { c->basew = size.min_width; c->baseh = size.min_height; }
	else c->basew = c->baseh = 0;
	c->incw = (c->flags & PResizeInc) ? size.width_inc : 0;
	c->inch = (c->flags & PResizeInc) ? size.height_inc : 0;
	c->maxw = (c->flags & PMaxSize) ? size.max_width : 0;
	c->maxh = (c->flags & PMaxSize) ? size.max_height : 0;
	c->minw = (c->flags & PMinSize) ? size.min_width : 0;
	c->minh = (c->flags & PMinSize) ? size.min_height : 0;
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh && c->maxw == c->minw && c->maxh == c->minh);
}

void updatetitle(Client * c) { if (!gettextprop(c->win, atom[WindowName], c->name, sizeof(c->name))) gettextprop(c->win, atom[WMName], c->name, sizeof(c->name)); }

int
xerror(Display * dsply, XErrorEvent * ee) {
	if (ee->error_code == BadWindow || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)) return 0;
	fprintf(stderr, "dechinus: fatal error: request code=%d, error code=%d\n", ee->request_code, ee->error_code);
	return xerrorxlib(dsply, ee);
}

int xerrordummy(Display * dsply, XErrorEvent * ee) { return 0; }
int xerrorstart(Display * dsply, XErrorEvent * ee) { otherwm = True; return -1; }

void
view(const char *arg) {
	unsigned int i = idxoftag(arg); Monitor *m, *cm = curmonitor(); if (!cm || cm->seltags[i]) return;
	memcpy(cm->prevtags, cm->seltags, ntags * sizeof(Bool));
	for (i=0; i<ntags; i++) cm->seltags[i] = (arg == NULL);
	cm->seltags[idxoftag(arg)] = True; cm->curtag = idxoftag(arg);
	updategeom(cm); arrange(cm); focus(NULL); updateatom[CurDesk] (NULL);
}

void
viewprevtag(const char *arg) {
	Bool tmptags[ntags]; unsigned int i = 0; Monitor *m = curmonitor(); if (!m) return;
	while (i < ntags - 1 && !m->prevtags[i]) i++;
	m->curtag = i;
	memcpy(tmptags, m->seltags, ntags * sizeof(Bool));
	memcpy(m->seltags, m->prevtags, ntags * sizeof(Bool));
	memcpy(m->prevtags, tmptags, ntags * sizeof(Bool));
	updategeom(m); arrange(m); focus(NULL); updateatom[CurDesk] (NULL);
}

void
viewlefttag(const char *arg) {
	unsigned int i; Monitor *m = curmonitor(); if (!m) return;
	for (i = 0; i < ntags; i++) if (i && m->seltags[i]) { view(tags[i - 1]); break; }
}

void
viewrighttag(const char *arg) {
	unsigned int i; Monitor *m = curmonitor(); if (!m) return;
	for (i = 0; i < ntags - 1; i++) if (m->seltags[i]) { view(tags[i + 1]); break; }
}

void
zoom(const char *arg) {
	Client *c; Monitor *m = curmonitor();
	if (!sel || !m || !FEATURES(views[m->curtag].layout, ZOOM) || sel->isfloating) return;
	if ((c = sel) == nexttiled(clients, m)) if (!(c = nexttiled(c->next, m))) return;
	detach(c); attach(c, 0); arrange(m); focus(c);
}

int
main(int argc, char *argv[]) {
	char conf[256] = "\0";
	if (argc == 3 && !strcmp("-f", argv[1])) snprintf(conf, sizeof(conf), "%s", argv[2]);
	else if (argc == 2 && !strcmp("-v", argv[1])) eprint("dechinus-" VERSION " (c) 2026 Daniel B. Prodigalidad\n");
	else if (argc != 1) eprint("usage: dewm [-v] [-f conf]\n");
	setlocale(LC_CTYPE, "");
	if (!(dpy = XOpenDisplay(0))) eprint("dechinus: cannot open display\n");
	signal(SIGHUP, sighandler); signal(SIGINT, sighandler); signal(SIGQUIT, sighandler);
	cargv = argv; screen = DefaultScreen(dpy); root = RootWindow(dpy, screen);
	checkotherwm(); setup(conf); scan(); run(); cleanup();
	XCloseDisplay(dpy); return 0;
}
