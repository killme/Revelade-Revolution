#include "engine.h"

static hashtable<const char *, font> fonts;
static font *fontdef = NULL;

font *curfont = NULL;
float gtextscale = 1.f;

void newfont(char *name, char *tex, int *canbedefault, int *defaultw, int *defaulth, int *offsetx, int *offsety) //, int *offsetw, int *offseth
{
    font *f = fonts.access(name);
    if(!f)
    {
        name = newstring(name);
        f = &fonts[name];
        f->name = name;
    }

    f->tex = textureload(tex);
    f->chars.shrink(0);
    f->charoffset = '!';
    f->defaultw = *defaultw;
    f->defaulth = *defaulth;
    f->offsetx = *offsetx;
    f->offsety = *offsety;
    //f->offsetw = *offsetw;
    //f->offseth = *offseth;
    f->offsetw = 0;
    f->offseth = 0;
    f->canbedefault = (*canbedefault)!=0;

    fontdef = f;
}

void fontoffset(char *c)
{
    if(!fontdef) return;

    fontdef->charoffset = c[0];
}

void fontchar(int *x, int *y, int *w, int *h)
{
    if(!fontdef) return;

    font::charinfo &c = fontdef->chars.add();
    c.x = *x;
    c.y = *y;
    c.w = *w ? *w : fontdef->defaultw;
    c.h = *h ? *h : fontdef->defaulth;
}

COMMANDN(font, newfont, "ssiiiii"); //ii
COMMAND(fontoffset, "s");
COMMAND(fontchar, "iiii");

bool setfont(const char *name)
{
    font *f = fonts.access(name);
    if(!f) return false;
    curfont = f;
    return true;
}

SVARFP(deffont, "digital", setfont(deffont));

ICOMMAND(enumfonts,"s",(char *last),{
    bool retnext = false;
    enumerate(fonts, font, f,
    //{
        if ((retnext || !last || last[0]=='\0') && f.canbedefault) { result(f.name); return; }
        else if (strcmp(f.name, last)==0)
        retnext = true;
    //}
    );
    result("");
})

static vector<font *> fontstack;

void pushfont()
{
    fontstack.add(curfont);
}

bool popfont()
{
    if(fontstack.empty()) return false;
    curfont = fontstack.pop();
    return true;
}

void gettextres(int &w, int &h)
{
    if(w < MINRESW || h < MINRESH)
    {
        if(MINRESW > w*MINRESH/h)
        {
            h = h*MINRESW/w;
            w = MINRESW;
        }
        else
        {
            w = w*MINRESH/h;
            h = MINRESH;
        }
    }
}

#define PIXELTAB (4*curfont->defaultw)

int text_width(const char *str) { //@TODO deprecate in favour of text_bounds(..)
    int width, height;
    text_bounds(str, width, height);
    return width;
}

void tabify(const char *str, int *numtabs)
{
    vector<char> tabbed;
    tabbed.put(str, strlen(str));
    int w = text_width(str), tw = max(*numtabs, 0)*PIXELTAB;
    while(w < tw)
    {
        tabbed.add('\t');
        w = ((w+PIXELTAB)/PIXELTAB)*PIXELTAB;
    }
    tabbed.add('\0');
    result(tabbed.getbuf());
}

COMMAND(tabify, "si");

void draw_textf(const char *fstr, int left, int top, ...)
{
    defvformatstring(str, top, fstr);
    draw_text(str, left, top);
}

static int draw_char(int c, int x, int y)
{
    font::charinfo &info = curfont->chars[c-curfont->charoffset];
    float tc_left    = (info.x + curfont->offsetx) / float(curfont->tex->xs);
    float tc_top     = (info.y + curfont->offsety) / float(curfont->tex->ys);
    float tc_right   = (info.x + info.w + curfont->offsetw) / float(curfont->tex->xs);
    float tc_bottom  = (info.y + info.h + curfont->offseth) / float(curfont->tex->ys);

    varray::attrib<float>(x,          y         ); varray::attrib<float>(tc_left,  tc_top   );
    varray::attrib<float>(x + info.w, y         ); varray::attrib<float>(tc_right, tc_top   );
    varray::attrib<float>(x + info.w, y + info.h); varray::attrib<float>(tc_right, tc_bottom);
    varray::attrib<float>(x,          y + info.h); varray::attrib<float>(tc_left,  tc_bottom);

    return info.w;
}

//stack[sp] is current color index
static void text_color(char c, char *stack, int size, int &sp, bvec color, int a)
{
    char d = c;
    if(d=='s') // save color
    {
        d = stack[sp];
        if(sp<size-1) stack[++sp] = d;
    }
    else
    {
        xtraverts += varray::end();
        if(d=='S') d = stack[(sp > 0) ? --sp : sp]; // restore color
        else stack[sp] = d;
        int f = a;
        switch(d)
        {
            case 'g': case '0': color = bvec( 64, 255,  64); break; // green: player talk
            case 'b': case '1': color = bvec(86,  92,  255); break; // blue: "echo" command
            case 'y': case '2': color = bvec(255, 255,   0); break; // yellow: gameplay messages
            case 'r': case '3': color = bvec(255,  64,  64); break; // red: important errors
            case 'a': case '4': color = bvec(192, 192, 192); break; // grey
            case 'm': case '5': color = bvec(255, 186, 255); break; // magenta
            case 'o': case '6': color = bvec(255,  64,   0); break; // orange
            case 'w': case '7': color = bvec(255, 255, 255); break; // white
            case 'k': case '8': color = bvec(0,     0,   0); break; // black
            case 'c': case '9': color = bvec(64,  255, 255); break; // cyan
            case 'v': color = bvec(192,  96, 255); break; // violet
            case 'p': color = bvec(224,  64, 224); break; // purple
            case 'n': color = bvec(164,  72,  56); break; // brown
            case 'G': color = bvec( 86, 164,  56); break; // dark green
            case 'B': color = bvec( 56,  64, 172); break; // dark blue
            case 'Y': color = bvec(172, 172,   0); break; // dark yellow
            case 'R': color = bvec(172,  56,  56); break; // dark red
            case 'M': color = bvec(172,  72, 172); break; // dark magenta
            case 'O': color = bvec(172,  56,   0); break; // dark orange
            case 'C': color = bvec(48,  172, 172); break; // dark cyan
            case 'A': case 'd': color = bvec(102, 102, 102); break; // dark grey
            case 'e': case 'E': f -= d!='E' ? f/2 : f/4; break;
            // provided color: everything else
        }
        glColor4ub(color.x, color.y, color.z, f);
    }
}

#define TEXTSKELETON \
    int y = 0, x = 0;\
    int i;\
    for(i = 0; str[i]; i++)\
    {\
        TEXTINDEX(i)\
        int c = str[i];\
        if(c=='\t')      { x = ((x+PIXELTAB)/PIXELTAB)*PIXELTAB*TEXTSCALE; TEXTWHITE(i) }\
        else if(c==' ')  { x += curfont->defaultw*TEXTSCALE; TEXTWHITE(i) }\
        else if(c=='\n') { TEXTLINE(i) x = 0; TEXTALIGN; }\
        else if(c=='\f') { if(str[i+1]) { i++; TEXTCOLOR(i) }}\
        else if(curfont->chars.inrange(c-curfont->charoffset))\
        {\
            if(maxwidth != -1)\
            {\
                int j = i;\
                int w = curfont->chars[c-curfont->charoffset].w*TEXTSCALE;\
                for(; str[i+1]; i++)\
                {\
                    int c = str[i+1];\
                    if(c=='\f') { if(str[i+2]) i++; continue; }\
                    if(i-j > 16) break;\
                    if(!curfont->chars.inrange(c-curfont->charoffset)) break;\
                    int cw = curfont->chars[c-curfont->charoffset].w*TEXTSCALE + 1;\
                    if(w + cw >= maxwidth) break;\
                    w += cw;\
                }\
                if(x + w >= maxwidth && j!=0) { TEXTLINE(j-1) x = 0; TEXTALIGN; }\
                TEXTWORD\
            }\
            else\
            { TEXTCHAR(i) }\
        }\
    }
#define TEXTALIGN \
    x = (!(flags&TEXT_RIGHT_JUSTIFY) && !(flags&TEXT_NO_INDENT) ? PIXELTAB : 0); \
    if(!y && (flags&TEXT_RIGHT_JUSTIFY) && !(flags&TEXT_NO_INDENT)) maxwidth -= PIXELTAB; \
    y += FONTH;
//all the chars are guaranteed to be either drawable or color commands
#define TEXTWORDSKELETON \
                for(; j <= i; j++)\
                {\
                    TEXTINDEX(j)\
                    int c = str[j];\
                    if(c=='\f') { if(str[j+1]) { j++; TEXTCOLOR(j) }}\
                    else { TEXTCHAR(j) }\
                }

int text_visible(const char *str, int hitx, int hity, int maxwidth, int flags)
{
    #define TEXTINDEX(idx)
    #define TEXTWHITE(idx) if(y+FONTH*(gtextscale) > hity && x >= hitx) return idx;
    #define TEXTLINE(idx) if(y+FONTH*(gtextscale) > hity) return idx;
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += curfont->chars[c-curfont->charoffset].w*(gtextscale)+1; TEXTWHITE(idx)
    #define TEXTWORD TEXTWORDSKELETON
    #define TEXTSCALE (gtextscale)
    TEXTSKELETON
    #undef TEXTSCALE
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
    return i;
}

//inverse of text_visible
void text_pos(const char *str, int cursor, int &cx, int &cy, int maxwidth, int flags)
{
    #define TEXTINDEX(idx) if(idx == cursor) { cx = x; cy = y; break; }
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx)
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += curfont->chars[c-curfont->charoffset].w*(gtextscale) + 1;
    #define TEXTWORD TEXTWORDSKELETON if(i >= cursor) break;
    #define TEXTSCALE (gtextscale)
    cx = INT_MIN;
    cy = 0;
    TEXTSKELETON
    if(cx == INT_MIN) { cx = x; cy = y; }
    #undef TEXTSCALE
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
}

void text_bounds(const char *str, int &width, int &height, int maxwidth, int flags)
{
    #define TEXTSCALE (gtextscale)
    #define TEXTINDEX(idx)
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx) if(x > width) width = x;
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += curfont->chars[c-curfont->charoffset].w*TEXTSCALE + 1;
    #define TEXTWORD x += w + 1;
    width = 0;
    if (maxwidth != -1) maxwidth /= TEXTSCALE;
    TEXTSKELETON
    height = y + FONTH*TEXTSCALE;
    TEXTLINE(_)
    #undef TEXTSCALE
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
}

int draw_text(const char *str, int left, int top, int r, int g, int b, int a, int flags, int cursor, int maxwidth)
{
    glPushMatrix();
    glTranslatef(left, top, 0.0f);
    glScalef(gtextscale, gtextscale, 1.0f);
    top = left = 0;
    if (maxwidth != -1) maxwidth /= gtextscale;

    #define TEXTINDEX(idx) if(idx == cursor) { cx = x; cy = y; }
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx)
    #define TEXTCOLOR(idx) text_color(str[idx], colorstack, sizeof(colorstack), colorpos, color, a);
    #define TEXTCHAR(idx) x += draw_char(c, left+x, top+y)+1;
    #define TEXTWORD TEXTWORDSKELETON
    #define TEXTSCALE (gtextscale)
    char colorstack[10];
    bvec color(r, g, b);
    int colorpos = 0, ly = 0, cx = INT_MIN, cy = 0;
    colorstack[0] = 'c'; //indicate user color
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, curfont->tex->id);
    glColor4ub(color.x, color.y, color.z, a);
    varray::enable();
    varray::defattrib(varray::ATTRIB_VERTEX, 2, GL_FLOAT);
    varray::defattrib(varray::ATTRIB_TEXCOORD0, 2, GL_FLOAT);
    varray::begin(GL_QUADS);
    TEXTSKELETON
    xtraverts += varray::end();
    if(cursor >= 0 && (totalmillis/250)&1)
    {
        glColor4ub(r, g, b, a);
        if(cx == INT_MIN) { cx = x; cy = y; }
        if(maxwidth != -1 && cx >= maxwidth) { cx = 0; cy += FONTH*TEXTSCALE; }
        draw_char('_', left+cx, top+cy);
        xtraverts += varray::end();
    }
    varray::disable();
    #undef TEXTSCALE
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD

    glPopMatrix();
    return ly + FONTH;
}

void reloadfonts()
{
    enumerate(fonts, font, f,
        if(!reloadtexture(*f.tex)) fatal("failed to reload font texture");
    );
}

int draw_textx(const char *fstr, int left, int top, int r, int g, int b, int a, int flags, int cursor, int maxwidth, ...)
{
    defvformatstring(str, maxwidth, fstr);

    int width = 0, height = 0;
    text_bounds(str, width, height, maxwidth);
    if(flags&TEXT_ALIGN) switch(flags&TEXT_ALIGN)
    {
        case TEXT_CENTERED: left -= width/2; break;
        case TEXT_RIGHT_JUSTIFY: left -= width; break;
        default: break;
    }
    if(flags&TEXT_UPWARD) top -= height;
    if(flags&TEXT_SHADOW) draw_text(str, left-2, top-2, 0, 0, 0, a, flags, cursor, maxwidth);
    return draw_text(str, left, top, r, g, b, a, flags, cursor, maxwidth);
}
