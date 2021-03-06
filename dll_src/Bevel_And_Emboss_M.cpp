#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <tuple>
#include <memory>
#include <vector>
#include <cmath>
#include <windows.h>
#include "lua.hpp"

struct Pixel_BGRA {
    uint8_t b, g, r, a;
};

struct Pixel_Info {
    int alpha;
    double dis;
    double x;
    double y;
    double line;
    double gray;
};

struct Point {
    double x, y;
};

struct ObjField {
    double ox, oy, oz;
    double rx, ry, rz;
    double cx, cy, cz;
    double zoom;
    double alpha;
    double aspect;
};

inline ObjField getObjField(lua_State* L) {
    ObjField obj;

    lua_getglobal(L, "obj");
    lua_getfield(L, -1, "ox");
    obj.ox = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "oy");
    obj.oy = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "oz");
    obj.oz = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "rx");
    obj.rx = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "ry");
    obj.ry = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "rz");
    obj.rz = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "cx");
    obj.cx = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "cy");
    obj.cy = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "cz");
    obj.cz = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "zoom");
    obj.zoom = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "alpha");
    obj.alpha = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "aspect");
    obj.aspect = lua_tonumber(L, -1);
    lua_pop(L, 1);

    return obj;
}

inline void setObjField(lua_State* L, ObjField obj) {
    lua_getglobal(L, "obj");
    lua_pushnumber(L, obj.ox);
    lua_setfield(L, -2, "ox");
    lua_pushnumber(L, obj.oy);
    lua_setfield(L, -2, "oy");
    lua_pushnumber(L, obj.oz);
    lua_setfield(L, -2, "oz");
    lua_pushnumber(L, obj.rx);
    lua_setfield(L, -2, "rx");
    lua_pushnumber(L, obj.ry);
    lua_setfield(L, -2, "ry");
    lua_pushnumber(L, obj.rz);
    lua_setfield(L, -2, "rz");
    lua_pushnumber(L, obj.cx);
    lua_setfield(L, -2, "cx");
    lua_pushnumber(L, obj.cy);
    lua_setfield(L, -2, "cy");
    lua_pushnumber(L, obj.cz);
    lua_setfield(L, -2, "cz");
    lua_pushnumber(L, obj.zoom);
    lua_setfield(L, -2, "zoom");
    lua_pushnumber(L, obj.alpha);
    lua_setfield(L, -2, "alpha");
    lua_pushnumber(L, obj.aspect);
    lua_setfield(L, -2, "aspect");
}

inline double rad(double deg) {
    return deg * 3.1415926535897932384626433832795028841971 / 180.0;
}

inline double squared(double num) {
    //std::pow(x, 2)???x??
    return num * num;
}

//?_[x,y]????????????????[x0,y0]:[x0+dx,y0+dy]?????_???A??????2???????W??????????
inline std::tuple<double, double, double> distance_line(double x, double y, double x0, double y0, double dx, double dy)
{
    //t<=0???????????A???????[?v???????????????????v?Z???K?v????
    double t = std::min(1.0, (dx * (x - x0) + dy * (y - y0)) / (dx * dx + dy * dy));
    //????????t<=0???????s?N?Z????if???e??????
    return std::make_tuple(std::abs(squared(x0 + dx * t - x) + squared(y0 + dy * t - y)), x0 + dx * t, y0 + dy * t);
}


int bevel_and_emboss(lua_State *L) {
    int a_th = lua_tointeger(L, 1);
    double bev_w = lua_tonumber(L, 2);
    double rot = lua_tonumber(L, 3);
    double high = lua_tonumber(L, 4);
    int style = lua_tointeger(L, 5);
    double blur = lua_tonumber(L, 6);
    int col1 = lua_tointeger(L, 7);
    int ble1 = lua_tointeger(L, 8);
    double alp1 = lua_tonumber(L, 9);
    int col2 = lua_tointeger(L, 10);
    int ble2 = lua_tointeger(L, 11);
    double alp2 = lua_tonumber(L, 12);
    int bgcol = lua_tointeger(L, 13);
    double preblur = lua_tonumber(L, 14);
    double isline = lua_tonumber(L, 15);

    if(style % 4 == 2 || style % 4 == 3)
        bev_w /= 2.0;

    if(bev_w <= 0 || style < 0 || style > 15)
        return 0;

    //4???T??????????????OutOfIndex???????????????}?[?W??&?????????????s???[???r??????????????
    lua_getglobal(L, "obj");
    lua_getfield(L, -1, "effect");
    lua_pushstring(L, "?????g??");
    lua_pushstring(L, "??");
    lua_pushnumber(L, preblur + 1);
    lua_pushstring(L, "??");
    lua_pushnumber(L, preblur + 1);
    lua_pushstring(L, "??");
    lua_pushnumber(L, preblur + 1);
    lua_pushstring(L, "?E");
    lua_pushnumber(L, preblur + 1);
    lua_call(L, 9, 0);

    if(style % 4 != 1) {
        lua_getfield(L, -1, "effect");
        lua_pushstring(L, "?????g??");
        lua_pushstring(L, "??");
        lua_pushnumber(L, std::ceil(bev_w));
        lua_pushstring(L, "??");
        lua_pushnumber(L, std::ceil(bev_w));
        lua_pushstring(L, "??");
        lua_pushnumber(L, std::ceil(bev_w));
        lua_pushstring(L, "?E");
        lua_pushnumber(L, std::ceil(bev_w));
        lua_call(L, 9, 0);
    }

    //"getpixeldata"??w,h?????????????I
    //???v???????Aexedit.auf???????O???????????????????????Agetpixel??????
    lua_getfield(L, -1, "getpixel");
    lua_call(L, 0, 2);
    int w = lua_tointeger(L, -2);
    int h = lua_tointeger(L, -1);
    lua_pop(L, 2);

    //?L???b?V???e?L?X?g?X?N???v?g?p?????O?????i?T?C?Y0???????????????????????G???[???N?????????j
    if(w * h == 0)
        return 0;
    
    lua_getfield(L, -1, "getpixeldata");
    lua_call(L, 0, 3);
    std::unique_ptr<Pixel_BGRA[]> bevel_buffer = std::make_unique<Pixel_BGRA[]>(w * h);
    memcpy(bevel_buffer.get(), lua_touserdata(L, -3), sizeof(Pixel_BGRA) * w * h);
    lua_pop(L, 3);

    //?????????????????????_?????????????????????Z
    lua_getfield(L, -1, "effect");
    lua_pushstring(L, "??????");
    lua_pushstring(L, "????");
    lua_pushnumber(L, preblur);
    lua_pushstring(L, "?T?C?Y????");
    lua_pushnumber(L, 1);
    lua_call(L, 5, 0);

    //?s?N?Z?????A???t?@?l,???s?????????Z????,???Z???s?????WX,Y,???s?????O?????l,?O???X?P(-1?`1)
    std::unique_ptr<Pixel_Info[]> pix_info = std::make_unique<Pixel_Info[]>(w * h);

    lua_getfield(L, -1, "getpixeldata");
    lua_call(L, 0, 3);

    Pixel_BGRA* pixel = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
    lua_pop(L, 3);
    std::unique_ptr<bool[]> border_ans = std::make_unique<bool[]>(w * h);
    
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            pix_info[j * w + i] = {pixel[j * w + i].a, HUGE_VAL, static_cast<double>(i), static_cast<double>(j), 1, 0};
            
            //rikky_module.bordering????????
            //?[???A?[???????????????????????E??th??????????????
            if(a_th < pixel[j * w + i].a && ((i == 0 || j == 0 || i == w-1 || j == h-1) || (pixel[j * w + i - 1].a <= a_th || pixel[j * w + i + 1].a <= a_th || pixel[j * w + i - w].a <= a_th || pixel[j * w + i + w].a <= a_th)))
                border_ans[j * w + i] = true;
            else
                border_ans[j * w + i] = false;
        }
    }

    //rikky_module.bordering
    std::vector<std::vector<Point>> p;
    while(1)
    {
        std::vector<Point> pb;
        Point tmp_point = {-1, -1};
        for(int i = 0; i < w * h; i++) {
            if(border_ans[i]) {
                tmp_point.x = i % w;
                tmp_point.y = i / w;
                break;
            }
        }
        if(tmp_point.x == -1 || tmp_point.y == -1)
            break;

        //?O??????
        bool ccw = (tmp_point.y == 0 || (pixel[static_cast<int>((tmp_point.y - 1) * w + tmp_point.x)].a <= a_th));
        
        while(true) {
            pb.push_back(tmp_point);
            border_ans[tmp_point.y * w + tmp_point.x] = false;
            Point next_point = {-1, -1};

            if(ccw) {
                //?????v???? ???????????v???????T??
                if(tmp_point.x != 0 && border_ans[tmp_point.y * w + tmp_point.x - 1])
                    next_point = {tmp_point.x - 1, tmp_point.y};
                else if(tmp_point.y != h - 1 && tmp_point.x != 0 &&  border_ans[(tmp_point.y + 1) * w + tmp_point.x - 1])
                    next_point = {tmp_point.x - 1, tmp_point.y + 1};
                else if(tmp_point.y != h - 1 && border_ans[(tmp_point.y + 1) * w + tmp_point.x])
                    next_point = {tmp_point.x, tmp_point.y + 1};
                else if(tmp_point.y != h - 1 && tmp_point.x != w - 1 && border_ans[(tmp_point.y + 1) * w + tmp_point.x + 1])
                    next_point = {tmp_point.x + 1, tmp_point.y + 1};
                else if(tmp_point.x != w - 1 && border_ans[tmp_point.y * w + tmp_point.x + 1])
                    next_point = {tmp_point.x + 1, tmp_point.y};
                else if(tmp_point.y != 0 && tmp_point.x != w - 1 && border_ans[(tmp_point.y - 1) * w + tmp_point.x + 1])
                    next_point = {tmp_point.x + 1, tmp_point.y - 1};
                else if(tmp_point.y != 0 && border_ans[(tmp_point.y - 1) * w + tmp_point.x] == 1)
                    next_point = {tmp_point.x, tmp_point.y - 1};
                else if(tmp_point.y != 0 && tmp_point.x != 0 &&  border_ans[(tmp_point.y - 1) * w + tmp_point.x - 1])
                    next_point = {tmp_point.x - 1, tmp_point.y - 1};
            } else {
                //???v???? ?E???????v???????T??
                if(tmp_point.x != w - 1 && border_ans[tmp_point.y * w + tmp_point.x + 1])
                    next_point = {tmp_point.x + 1, tmp_point.y};
                else if(tmp_point.y != h - 1 && tmp_point.x != w - 1 && border_ans[(tmp_point.y + 1) * w + tmp_point.x + 1])
                    next_point = {tmp_point.x + 1, tmp_point.y + 1};
                else if(tmp_point.y != h - 1 && border_ans[(tmp_point.y + 1) * w + tmp_point.x])
                    next_point = {tmp_point.x, tmp_point.y + 1};
                else if(tmp_point.y != h - 1 && tmp_point.x != 0 &&  border_ans[(tmp_point.y + 1) * w + tmp_point.x - 1])
                    next_point = {tmp_point.x - 1, tmp_point.y + 1};
                else if(tmp_point.x != 0 && border_ans[tmp_point.y * w + tmp_point.x - 1])
                    next_point = {tmp_point.x - 1, tmp_point.y};
                else if(tmp_point.y != 0 && tmp_point.x != 0 &&  border_ans[(tmp_point.y - 1) * w + tmp_point.x - 1])
                    next_point = {tmp_point.x - 1, tmp_point.y - 1};
                else if(tmp_point.y != 0 && border_ans[(tmp_point.y - 1) * w + tmp_point.x])
                    next_point = {tmp_point.x, tmp_point.y - 1};
                else if(tmp_point.y != 0 && tmp_point.x != w - 1 && border_ans[(tmp_point.y - 1) * w + tmp_point.x + 1])
                    next_point = {tmp_point.x + 1, tmp_point.y - 1};
            }

            if(next_point.x == -1 || next_point.y == -1)
                break;
            
            tmp_point = next_point;
        }        
        p.push_back(pb);
    }

    lua_getfield(L, -1, "putpixeldata");
    lua_pushlightuserdata(L, bevel_buffer.get());
    lua_call(L, 1, 0);

    //???T???????x???Q???????A???s???????_??????????
    for(int i = 0; i < p.size(); i++) {
        for(int j = p[i].size() - 1; j >= 0; j--) {
            Point point = p[i][j];
            int index = point.y * w + point.x;
            double a5 = pix_info[index].alpha, a6 = pix_info[index + 1].alpha, a4 = pix_info[index - 1].alpha, a2 = pix_info[index + w].alpha, a8 = pix_info[index - w].alpha;
            //???????S?????????l???????s?N?Z???????X????????????
            if(std::min({a5, a6, a4, a2, a8}) >= a_th) {
                //vector.erase???????d???????v????????
                p[i].erase(p[i].begin() + j);
            } else {
                double dx = std::max(-1.0, std::min(1.0, a6 < a_th ? (a5 - a_th) / (a5 - a6) : a4 < a_th ? (a5 - a_th) / (a4 - a5) : 0.0));
                double dy = std::max(-1.0, std::min(1.0, a2 < a_th ? (a5 - a_th) / (a5 - a2) : a8 < a_th ? (a5 - a_th) / (a8 - a5) : 0.0));
                p[i][j].x += dx / (1 + std::abs(dy / dx)); 
                p[i][j].y += dy / (1 + std::abs(dx / dy));
            }
        }
    }

    //???s??????????
    //?????????i????????)
    double hoge = 0.8;
    for(int i = 0; i < p.size(); i++) {
        double bufx = p[i][0].x, bufy = p[i][0].y;
        for(int j = 1; j < p[i].size(); j++) {
            double x = p[i][j].x, y = p[i][j].y;
            if(squared(x * 2 - bufx - p[i][(j + 1) % p[i].size()].x) + squared(y * 2 - bufy - p[i][(j + 1) % p[i].size()].y) < hoge) {
                p[i][j].x = x / 2 + bufx / 4 + p[i][(j + 1) % p[i].size()].x / 4;
                p[i][j].y = y / 2 + bufy / 4 + p[i][(j + 1) % p[i].size()].y / 4;
            }
            bufx, bufy = x, y;
        }
    }

    for(int i = 0; i < p.size(); i++) {
        for(int j = p[i].size() - 3; j >= 0; j--) {
            double x = p[i][j].x, y = p[i][j].y;
            double xn = p[i][j+1].x, yn = p[i][j+1].y;
            double xnn = p[i][j+2].x, ynn = p[i][j+2].y;
            //?O???????l?????i?????????????????????j?????????_??????
            if(std::abs((x-xn)*(yn-ynn)-(y-yn)*(xn-xnn)) <= isline) {
                //vector.erase???????d???????v????????
                p[i].erase(p[i].begin() + j + 1);
            }
        }
    }
    //?????????????????????s?????g???????????????????A?????????????????????????????o???????v??

    for(int i = 0; i < p.size(); i++) {
        for(int j = 0; j < p[i].size(); j++) {
            double x0 = p[i][j].x, y0 = p[i][j].y, x1 = p[i][(j + 1) % p[i].size()].x, y1  = p[i][(j + 1) % p[i].size()].y;
            double dx = x1 - x0, dy = y1 - y0;
            int xmin = std::floor(std::max(1.0,std::min(x0,x1)-bev_w));
            int xmax = std::ceil(std::min(w-1.0,std::max(x0,x1)+bev_w));
            int ymin = std::floor(std::max(1.0,std::min(y0,y1)-bev_w));
            int ymax = std::ceil(std::min(h-1.0,std::max(y0,y1)+bev_w));
            for(int y = ymin; y <= ymax; y++) {
                for(int x = xmin; x <= xmax; x++) {
                    double x_ul = x - 0.5, y_ul = y - 0.5;
                    //?|?C???^?n??
                    Pixel_Info* info = &pix_info.get()[y*w+x];
                    //[x0,y0]???????????????????????v?Z???K?v?????i???????[?v???v?Z???????j
                    if(dx * (x_ul - x0) > dy * (y0 - y_ul)) {
                        std::tuple<double, double, double> ret = distance_line(x_ul, y_ul, x0, y0, dx, dy);
                        //???r?i?K????math.sqrt()???g???K?v??????
                        if(info->dis > std::get<0>(ret)) {
                            info->dis = std::get<0>(ret);
                            info->x = std::get<1>(ret);
                            info->y = std::get<2>(ret);
                            //???s???????O?????i?^???l?????????????????Z?????????????????A???R???j
                            info->line = dy * (x_ul - x0) - dx * (y_ul - y0);
                        }
                    }
                }
            }
        }
    }

    //?????????????????g???????`??????
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            Pixel_Info* info = &pix_info[j * w + i];
            info->dis = std::sqrt(info->dis);
            info->x = (info->x - (i - 0.5)) / info->dis;
            info->y = (info->y - (j - 0.5)) / info->dis;
            info->gray = std::cos(std::atan2(info->y, info->x) + rad(rot)) * std::cos(rad(high)) * (info->line < 0 ? -1 : 1);
        }
    }

    //?l?????F???????S???F?????????A???`?G?C???A?X????
    for(int j = 1; j < h - 1; j++) {
        for(int i = 1; i < w - 1; i++) {
            Pixel_Info* info7 = &pix_info[j * w + i];
            Pixel_Info* info9 = &pix_info[j * w + i + 1];
            Pixel_Info* info1 = &pix_info[j * w + i + w];
            Pixel_Info* info3 = &pix_info[j * w + i + 1 + w];
            double p8 = info7->x == info9->x ? 0.5 : (std::min(1.0, std::max(0.0, (info9->dis - info7->dis + info9->x) / (info9->x - info7->x))));
            double p4 = info7->y == info1->y ? 0.5 : (std::min(1.0, std::max(0.0, (info1->dis - info7->dis + info1->y) / (info1->y - info7->y))));
            double p6 = info9->y == info3->y ? 0.5 : (std::min(1.0, std::max(0.0, (info3->dis - info9->dis + info3->y) / (info3->y - info9->y))));
            double p2 = info1->x == info3->x ? 0.5 : (std::min(1.0, std::max(0.0, (info3->dis - info1->dis + info3->x) / (info3->x - info1->x))));
            //4???_???????l???????R?X?g
            double x = (p8 + p2 + 1) / 4, y = (p4 + p6 + 1) / 4;
            info7->gray = (info7->gray * (p8 * y + p4 * x) + info9->gray * ((1 - p8) * y + p6 * (1 - x)) + info1->gray * ((1 - p4) * x + p2 * (1 - y)) + info3->gray * ((1 - p6) * (1 - x) + (1 - p2) * (1 - y))) / 2;
            //?????????????s?N?Z?????????????W???v?Z???????????A?G??????????
            info7->dis = (info7->dis + info9->dis + info1->dis + info3->dis) / 4.0;
        }
    }

    switch(style) {
        case 0: //?x?x??(?O??)
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].r = (bgcol >> 16) & 0xff;
                pix[i].g = (bgcol >> 8) & 0xff;
                pix[i].b = bgcol & 0xff;
                pix[i].a = 0xff;
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);
            
            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "tmp");
            lua_pushstring(L, "obj");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].r = (col1 >> 16) & 0xff;
                pix[i].g = (col1 >> 8) & 0xff;
                pix[i].b = col1 & 0xff;
                pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, pix_info[i].gray));
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble1);
            lua_call(L, 2, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp1 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].r = (col2 >> 16) & 0xff;
                pix[i].g = (col2 >> 8) & 0xff;
                pix[i].b = col2 & 0xff;
                pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, -pix_info[i].gray));
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble2);
            lua_call(L, 2, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp2 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].r = static_cast<uint8_t>(bevel_buffer[i].r * (bevel_buffer[i].a / 255.0) + pix[i].r * (1.0 - bevel_buffer[i].a / 255.0));
                pix[i].g = static_cast<uint8_t>(bevel_buffer[i].g * (bevel_buffer[i].a / 255.0) + pix[i].g * (1.0 - bevel_buffer[i].a / 255.0));
                pix[i].b = static_cast<uint8_t>(bevel_buffer[i].b * (bevel_buffer[i].a / 255.0) + pix[i].b * (1.0 - bevel_buffer[i].a / 255.0));
                pix[i].a = std::max(bevel_buffer[i].a, static_cast<uint8_t>(std::max(std::min(1.0, bev_w - pix_info[i].dis), 0.0) * 255));
            }
            
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            break;
        }
        case 1: //?x?x??(????)
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].a = 0xff;
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);
            
            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "tmp");
            lua_pushstring(L, "obj");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble1);
            lua_call(L, 2, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp1 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, -pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble2);
            lua_call(L, 2, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp2 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].a = bevel_buffer[i].a;
            }
            
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            break;
        }
        case 2: //?G???{?X
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].r = static_cast<uint8_t>(pix[i].r * (pix[i].a / 255.0) + ((bgcol>>16) & 0xff) * (1 - pix[i].a / 255.0));
                pix[i].g = static_cast<uint8_t>(pix[i].g * (pix[i].a / 255.0) + ((bgcol>>8) & 0xff) * (1 - pix[i].a / 255.0));
                pix[i].b = static_cast<uint8_t>(pix[i].b * (pix[i].a / 255.0) + (bgcol & 0xff) * (1 - pix[i].a / 255.0));
                pix[i].a = 0xff;
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);
            
            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "tmp");
            lua_pushstring(L, "obj");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, pix_info[i].gray) * (pix_info[i].line < 0 ? 1 : std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis))));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble1);
            lua_call(L, 2, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp1 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, -pix_info[i].gray) * (pix_info[i].line < 0 ? 1 : std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis))));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble2);
            lua_call(L, 2, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp2 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].a = std::max(bevel_buffer[i].a, static_cast<uint8_t>(std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)) * 255.0));
            }
            
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            break;
        }
        case 3: //?s???[?G???{?X
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].r = static_cast<uint8_t>(pix[i].r * (pix[i].a / 255.0) + ((bgcol>>16) & 0xff) * (1 - pix[i].a / 255.0));
                pix[i].g = static_cast<uint8_t>(pix[i].g * (pix[i].a / 255.0) + ((bgcol>>8) & 0xff) * (1 - pix[i].a / 255.0));
                pix[i].b = static_cast<uint8_t>(pix[i].b * (pix[i].a / 255.0) + (bgcol & 0xff) * (1 - pix[i].a / 255.0));
                pix[i].a = 0xff;
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);
            
            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "tmp");
            lua_pushstring(L, "obj");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, (bevel_buffer[i].a * 2.0 / 255.0 - 1.0) * pix_info[i].gray) * (pix_info[i].line < 0 ? 1 : std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis))));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble1);
            lua_call(L, 2, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp1 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, -(bevel_buffer[i].a * 2.0 / 255.0 - 1.0) * pix_info[i].gray) * (pix_info[i].line < 0 ? 1 : std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis))));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble2);
            lua_call(L, 2, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp2 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                pix[i].a = std::max(bevel_buffer[i].a, static_cast<uint8_t>(std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)) * 255.0));
            }
            
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            break;
        }
        case 4: //?x?x??(?O??)?????`??
        {
            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            ObjField obj = getObjField(L);
            lua_getfield(L, -1, "draw");
            lua_call(L, 0, 0);

            //obj.effect()???????x???????????\??????????????????????
            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_pushnumber(L, w);
            lua_pushnumber(L, h);
            lua_call(L, 4, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);
            
            setObjField(L, obj);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(std::min(255.0 - bevel_buffer[i].a, 255.0 * std::max(0.0, pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble1);
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp1 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_pushnumber(L, w);
            lua_pushnumber(L, h);
            lua_call(L, 4, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);
            
            setObjField(L, obj);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(std::min(255.0 - bevel_buffer[i].a, 255.0 * std::max(0.0, -pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble2);
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp2 / 100.0);
            lua_call(L, 5, 0);
            
            break;
        }
        case 5: //?x?x??(????)?????`??
        {
            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            ObjField obj = getObjField(L);
            lua_getfield(L, -1, "draw");
            lua_call(L, 0, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_pushnumber(L, w);
            lua_pushnumber(L, h);
            lua_call(L, 4, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);
            
            setObjField(L, obj);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(std::min(static_cast<double>(bevel_buffer[i].a), 255.0 * std::max(0.0, pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble1);
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp1 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_pushnumber(L, w);
            lua_pushnumber(L, h);
            lua_call(L, 4, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);
            
            setObjField(L, obj);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(std::min(static_cast<double>(bevel_buffer[i].a), 255.0 * std::max(0.0, -pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble2);
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp2 / 100.0);
            lua_call(L, 5, 0);
            break;
        }
        case 6: //?G???{?X ?????`??
        {
            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            ObjField obj = getObjField(L);
            lua_getfield(L, -1, "draw");
            lua_call(L, 0, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_pushnumber(L, w);
            lua_pushnumber(L, h);
            lua_call(L, 4, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);
            
            setObjField(L, obj);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, pix_info[i].gray) * std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble1);
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp1 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_pushnumber(L, w);
            lua_pushnumber(L, h);
            lua_call(L, 4, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);
            
            setObjField(L, obj);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, -pix_info[i].gray) * std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble2);
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp2 / 100.0);
            lua_call(L, 5, 0);
            break;
        }
        case 7: //?s???[?G???{?X ?????`??
        {
            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            ObjField obj = getObjField(L);
            lua_getfield(L, -1, "draw");
            lua_call(L, 0, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_pushnumber(L, w);
            lua_pushnumber(L, h);
            lua_call(L, 4, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);
            
            setObjField(L, obj);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, (bevel_buffer[i].a * 2 / 255.0 - 1) * pix_info[i].gray) * std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble1);
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp1 / 100.0);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "tempbuffer");
            lua_pushnumber(L, w);
            lua_pushnumber(L, h);
            lua_call(L, 4, 0);

            lua_getfield(L, -1, "copybuffer");
            lua_pushstring(L, "obj");
            lua_pushstring(L, "tmp");
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "drawtarget");
            lua_pushstring(L, "framebuffer");
            lua_call(L, 2, 0);
            
            setObjField(L, obj);

            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, (bevel_buffer[i].a * 2 / 255.0 - 1) * (-pix_info[i].gray)) * std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);

            lua_getfield(L, -1, "setoption");
            lua_pushstring(L, "blend");
            lua_pushnumber(L, ble2);
            lua_call(L, 2, 0);

            lua_getfield(L, -1, "effect");
            lua_call(L, 0, 0);
            
            lua_getfield(L, -1, "draw");
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, alp2 / 100.0);
            lua_call(L, 5, 0);
            break;
        }
        case 8: //?x?x??(?O??) ?n?C???C?g????
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(std::min(255.0 - bevel_buffer[i].a, 255.0 * std::max(0.0, pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);
            break;
        }
        case 9: //?x?x??(????) ?n?C???C?g????
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(std::min(static_cast<double>(bevel_buffer[i].a), 255.0 * std::max(0.0, pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);
            break;
        }
        case 10: //?G???{?X ?n?C???C?g????
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, pix_info[i].gray) * std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);
            break;
        }
        case 11: //?s???[?G???{?X ?n?C???C?g????
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col1 >> 16) & 0xff;
                    pix[i].g = (col1 >> 8) & 0xff;
                    pix[i].b = col1 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, (bevel_buffer[i].a * 2 / 255.0 - 1) * pix_info[i].gray) * std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);
            break;
        }
        case 12: //?x?x??(?O??) ?V???h?E????
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(std::min(255.0 - bevel_buffer[i].a, 255.0 * std::max(0.0, -pix_info[i].gray) * std::min(1.0, bev_w-pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);
            break;
        }
        case 13: //?x?x??(????) ?V???h?E????
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(std::min(static_cast<double>(bevel_buffer[i].a), 255.0 * std::max(0.0, -pix_info[i].gray) * std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);
            break;
        }
        case 14: //?G???{?X ?V???h?E????
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, -pix_info[i].gray) * std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);
            break;
        }
        case 15: //?s???[?G???{?X ?V???h?E????
        {
            lua_getfield(L, -1, "getpixeldata");
            lua_call(L, 0, 3);
            Pixel_BGRA* pix = reinterpret_cast<Pixel_BGRA*>(lua_touserdata(L, -3));
            lua_pop(L, 3);
            for(int i = 0; i < w * h; i++) {
                if(pix_info[i].dis <= bev_w) {
                    pix[i].r = (col2 >> 16) & 0xff;
                    pix[i].g = (col2 >> 8) & 0xff;
                    pix[i].b = col2 & 0xff;
                    pix[i].a = static_cast<uint8_t>(255.0 * std::max(0.0, (bevel_buffer[i].a * 2 / 255.0 - 1) * (-pix_info[i].gray)) * std::max(0.0, std::min(1.0, bev_w - pix_info[i].dis)));
                } else {
                    pix[i].r = 0;
                    pix[i].g = 0;
                    pix[i].b = 0;
                    pix[i].a = 0;
                }
            }
            lua_getfield(L, -1, "putpixeldata");
            lua_pushlightuserdata(L, pix);
            lua_call(L, 1, 0);

            lua_getfield(L, -1, "effect");
            lua_pushstring(L, "??????");
            lua_pushstring(L, "????");
            lua_pushnumber(L, blur);
            lua_pushstring(L, "?T?C?Y????");
            lua_pushnumber(L, 1);
            lua_call(L, 5, 0);
            break;
        }
    }

    lua_getfield(L, -1, "setoption");
    lua_pushstring(L, "blend");
    lua_pushnumber(L, 0);
    lua_call(L, 2, 0);

    lua_getfield(L, -1, "setoption");
    lua_pushstring(L, "drawtarget");
    lua_pushstring(L, "framebuffer");
    lua_call(L, 2, 0);

    return 0;
}

static luaL_Reg functions[] = {
    { "Bevel_And_Emboss", bevel_and_emboss },
    { nullptr, nullptr }
};

EXTERN_C __declspec(dllexport) int luaopen_Bevel_And_Emboss_M(lua_State* L) {
    luaL_register(L, "Bevel_And_Emboss_M", functions);
    return 1;
}
