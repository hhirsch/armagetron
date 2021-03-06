/*

*************************************************************************

ArmageTron -- Just another Tron Lightcycle Game in 3D.
Copyright (C) 2000  Manuel Moos (manuel@moosnet.de)

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

***************************************************************************

*/
#include "cockpit/cMap.h"
#include "cockpit/cCockpit.h"
#include "nConfig.h"
#include "tCoord.h"

#ifndef DEDICATED

#include "rRender.h"
#include "rScreen.h"
#include "gWinZone.h"
#include "eRectangle.h"
#include "ePlayer.h"
#include "eTimer.h"

#include "rViewport.h"
#include "eGrid.h"
#include "gCycle.h"

#endif

#include <vector>

static bool stc_forbidHudMap = false;
// TODO find out why this is throwing an error: “Two tConfItems with the same name FORBID_HUD_MAP!”
// Z-Man: hmm, it doesn't throw an error for me.
static nSettingItem<bool> fcs("FORBID_HUD_MAP", stc_forbidHudMap);
extern std::vector<tCoord> se_rimWallRubberBand;

#ifndef DEDICATED

extern std::deque<gZone *> sg_Zones;

namespace cWidget {

void Map::ClipperRect::Begin(Map &map, tCoord const &e1, tCoord const &e2) {
    // set clipping frame in map coordinates
    GLdouble pl0[4] = {1.0, 0.0, 0.0, -e1.x };
    GLdouble pl1[4] = {-1.0, 0.0, 0.0, e2.x };
    GLdouble pl2[4] = {0.0, 1.0, 0.0, -e1.y };
    GLdouble pl3[4] = {0.0, -1.0, 0.0, e2.y };
    glClipPlane(GL_CLIP_PLANE0, pl0);
    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE1, pl1);
    glEnable(GL_CLIP_PLANE1);
    glClipPlane(GL_CLIP_PLANE2, pl2);
    glEnable(GL_CLIP_PLANE2);
    glClipPlane(GL_CLIP_PLANE3, pl3);
    glEnable(GL_CLIP_PLANE3);
    // Add frame ...
    map.m_foreground.BeginDraw();
    glBegin(GL_LINE_STRIP);
    //TODO: this should use a function of the rGradient.
    map.m_foreground.DrawPoint(e1);
    map.m_foreground.DrawPoint(tCoord(e2.x, e1.y));
    map.m_foreground.DrawPoint(e2);
    map.m_foreground.DrawPoint(tCoord(e1.x, e2.y));
    map.m_foreground.DrawPoint(e1);
    glEnd();
    map.m_background.SetGradientEdges(e1, e2);
    map.m_background.DrawRect(e1, e2);
}

void Map::ClipperRect::End() {
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
}

Map::ClipperCircle::ClipperCircle() {
    glGetIntegerv(GL_MAX_CLIP_PLANES, (GLint *)(&m_edges));
    if(m_edges > 20) {
        m_edges = 20;
    }
}
//the visible area is on the "left side" of the line, if you go from u to v
void Map::ClipperCircle::Clip(int i, tCoord const &u, tCoord const &v) {
    //glBegin(GL_LINES);
    //Color(1,0,0);
    //Vertex(u.x, u.y);
    //Color(0,1,0);
    //Vertex(v.x, v.y);
    //glEnd();
    if(u.x == v.x) {
        GLdouble factor = (u.y>v.y) ? 1 : -1;
        GLdouble pl[4] = {factor*1.0, 0.0, 0.0, -factor*v.x };
        glClipPlane(GL_CLIP_PLANE0+i, pl);
        glEnable(GL_CLIP_PLANE0+i);
    } else {
        GLdouble factor = (u.x<v.x) ? -1 : 1;
        GLdouble a = (v.y-u.y)/(v.x-u.x);
        GLdouble pl[4] = {
                             factor*-a,
                             factor,
                             0.,
                             factor*-(u.y-a*u.x)
                         };
        glClipPlane(GL_CLIP_PLANE0+i, pl);
        glEnable(GL_CLIP_PLANE0+i);
    }
}
void Map::ClipperCircle::Begin(Map &map, tCoord const &e1, tCoord const &e2) {
    tCoord centre = .5*(e1+e2);
    tCoord ab = .5*(e2-e1);
    ab.x = fabs(ab.x); ab.y = fabs(ab.y);
    float stepsize=M_PI*2/m_edges;

	map.m_background.BeginDraw();
	map.m_background.SetGradientEdges(centre - ab, centre + ab);
	glBegin(GL_POLYGON);
    for(int i = 0; i < m_edges; ++i) {
        float t = (i+1)*stepsize;
        tCoord next(centre.x+ab.x*cos(t), centre.y-ab.y*sin(t));
        map.m_background.DrawPoint(next);
    }
	glEnd();
	glDisable(GL_TEXTURE_2D);
    tCoord last = centre + tCoord(ab.x, 0);
	map.m_foreground.SetGradientEdges(centre - ab, centre + ab);
	map.m_foreground.BeginDraw();
    for(int i = 0; i < m_edges; ++i) {
        float t = (i+1)*stepsize;
        tCoord next(centre.x+ab.x*cos(t), centre.y-ab.y*sin(t));
		glBegin(GL_LINES);
        glVertex2f(next.x, next.y);
        glVertex2f(last.x, last.y);
        map.m_foreground.DrawPoint(next);
        map.m_foreground.DrawPoint(last);
		glEnd();
        Clip(i, last, next);
        last = next;
    }
	glDisable(GL_TEXTURE_2D);
}
void Map::ClipperCircle::End() {
    for(int i = 0; i < m_edges; ++i) {
        glDisable(GL_CLIP_PLANE0 + i); //according to the documentation you're allowed to do that
    }
}

bool Map::Process(tXmlParser::node cur) {
    if (
        WithCoordinates ::Process(cur) ||
        WithForeground ::Process(cur) ||
        WithBackground ::Process(cur))
        return true;
    if(cur.IsOfType("MapModes")) {
        int toggleKey;
        cur.GetProp("toggleKey", toggleKey);
        if(toggleKey > 0) {
            m_toggleKey = toggleKey;
            m_Cockpit->AddEventHandler(toggleKey, this);
        }
        for(cur = cur.GetFirstChild(); cur; ++cur) {
            if(cur.IsOfType("MapMode")) {
                m_modes.push_back(Mode(cur));
                if(m_modes.size() == 1) {
                    Apply(m_modes[0]); // apply the first mode and get rid of the defaults
                }
            }
        }
        return true;
    }
    DisplayError(cur);
    return false;
}
Map::Mode::Mode(tXmlParser::node cur) {
    cur.GetProp("zoomFactor", m_zoom);
    tString mode = cur.GetProp("mode");
    if(mode == "closestZone") m_mode = MODE_ZONE;
    else if(mode == "cycle") m_mode = MODE_CYCLE;
    else m_mode = MODE_STD;

    tString rotation = cur.GetProp("rotation");
    if(rotation == "fixed") m_rotation = ROTATION_FIXED;
    else if(rotation == "cycle") m_rotation = ROTATION_CYCLE;
    else if(rotation == "camera") m_rotation = ROTATION_CAMERA;
    else m_rotation = ROTATION_SPAWN;

    tString clipper = cur.GetProp("clipMode");
    if(clipper == "ellipse") {
        m_clipper = ClipperCircle::create;
    } else {
        m_clipper = ClipperRect::create;
    }
}
void Map::Apply(Mode const &mode) {
    m_mode = mode.m_mode;
    m_rotation = mode.m_rotation;
    m_zoom = mode.m_zoom;
    m_clipper.reset((*mode.m_clipper)());
}
void Map::HandleEvent(bool state, int id) {
    if(id == m_toggleKey) {
        if(state) {
            ToggleMode();
        }
    } else {
        Base::HandleEvent(state, id);
    }
}

void Map::Render() {
    // I haven't checked possible initial matrix state, so init to identity and modelview
    if(stc_forbidHudMap) return; // the server doesn't want us to do that
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_LINE_SMOOTH);
    glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);
    DrawMap(true, true,
            5.5, 0.,
            m_position.x-m_size.x, m_position.y-m_size.y, 2.*m_size.x, 2.*m_size.y,
            sr_screenWidth*m_size.x, sr_screenWidth*m_size.y, .5, .5);
}
void Map::DrawMap(bool rimWalls, bool cycleWalls,
                  double cycleSize, double border,
                  double x, double y, double w, double h,
                  double rw, double rh, double ix, double iy) {
    double pl_CurSpeed, min_dist2, dist2, rad, zoom = 1;
    tCoord pl_CurPos, rotate; // rotate will hold the cos and sin of the rotation to apply
    cCockpit* cp = m_Cockpit;
    if(!rimWalls && !cycleWalls) return;
    const eRectangle &bounds = eWallRim::GetBounds();
    double lx = bounds.GetLow().x - border, hx = bounds.GetHigh().x + border;
    double ly = bounds.GetLow().y - border, hy = bounds.GetHigh().y + border;
    double mw = hx - lx, mh = hy - ly;
    double xpos, ypos, xscale, yscale;
    double xrat = (rw * mh) / (rh * mw);
    double yrat = (rh * mw) / (rw * mh);
    // set scale and position
    if(xrat > yrat) {
        xscale = (w * rh) / (mh * rw);
        yscale = h / mh;
    } else {
        xscale = w / mw;
        yscale = (h * rw) / (mw * rh);
    }
    rotate.x = 1; // no rotation
    rotate.y = 0;

    //do the rotation
    switch(m_rotation) {
    case ROTATION_SPAWN:
    if(!cp->GetFocusCycle()) { break; }
        rotate = cp->GetFocusCycle()->SpawnDirection().Turn(0,-1);
        break;
    case ROTATION_CYCLE:
    if(!cp->GetFocusCycle()) { break; }
        rotate = cp->GetFocusCycle()->Direction().Turn(0,-1);
        break;
    case ROTATION_CAMERA:
        {
            ePlayer const *player = cp->GetPlayer();
            if(!player) break;
            eCamera const *cam = player->cam;
            if(cam) {
                rotate = cam->CameraDir().Turn(0,-1);
            } else {
                rotate = tCoord(1, 0);
            }
        }
        break;
    default:
        rotate = tCoord(1, 0);
        break;
    }

    // manage mode
    switch (m_mode) {
    case MODE_ZONE:
    if(!cp->GetFocusCycle()) { break; }
        pl_CurPos = cp->GetFocusCycle()->Position();
        min_dist2 = (mw*mw+mh*mh)*1000;
        rad = 0;
        for(std::deque<gZone *>::const_iterator i = sg_Zones.begin(); i != sg_Zones.end(); ++i) {
            tASSERT(*i);
            tCoord const &position = (*i)->GetPosition();
            const float radius = (*i)->GetRadius();
            dist2 = (position-pl_CurPos).Norm();
            if (dist2<min_dist2) {
                min_dist2 = dist2;
                m_centre = position;
                rad = radius;
            }
        }
        rad = (rad<15)?15:rad;
        zoom = (w>h)?h/(yscale*m_zoom*rad):w/(xscale*m_zoom*rad);
        zoom = (zoom<1)?1:zoom;
        // check if at least 1 zone was found, if not, toggle to mode 2 ...
        if (rad==0) {
            m_mode = MODE_CYCLE;
        } else {
            break;
        }
    case MODE_CYCLE:
    if(!cp->GetFocusCycle()) { break; }
        m_centre = cp->GetFocusCycle()->Position();
        pl_CurSpeed = cp->GetFocusCycle()->Speed();
        zoom = (w>h)?h/(yscale*m_zoom*pl_CurSpeed):w/(xscale*m_zoom*pl_CurSpeed);
        zoom = (zoom<1)?1:zoom;
        break;
    default :
        zoom = 1;
        m_centre.x = lx + mw / 2;
        m_centre.y = ly + mh / 2;
        break;
    }
    xscale *=zoom;
    yscale *=zoom;
    xpos = x - m_centre.x * xscale + w / 2;
    ypos = y - m_centre.y * yscale + h / 2;
    if(m_mode != MODE_STD) {
        m_clipper->Begin(*this, tCoord(x,y), tCoord(x+w, y+h));
    }
    // set projection matrix
    glPushMatrix();
    glTranslatef(xpos, ypos, 0);
    glScalef(xscale, yscale, 1);
    // translate and rotate
    GLfloat r[16] = {
          rotate.x,  -rotate.y, 0, 0,
          rotate.y,   rotate.x, 0, 0,
                 0,          0, 1, 0,
        m_centre.x, m_centre.y, 0, 1};
    glMultMatrixf(r);
    glTranslatef(-m_centre.x,-m_centre.y,0);
    if(rimWalls)
        DrawRimWalls(se_rimWalls);
    if(cycleWalls) {
        DrawWalls(sg_netPlayerWallsGridded);
        DrawWalls(sg_netPlayerWalls);
    }
    DrawObjects(tCoord((cycleSize * w) / (rw * xscale), (cycleSize * h) / (rh * yscale)));
    glPopMatrix();
    if(m_mode != MODE_STD) {
        m_clipper->End();
    }
}

void Map::DrawRimWalls( tList<eWallRim> &list ) {
    if(sr_alphaBlend && m_mode == MODE_STD) {
        const eRectangle &bounds = eWallRim::GetBounds();
        const tCoord dims = bounds.GetHigh() - bounds.GetLow();
        const float max = fmax(dims.x, dims.y); // make sure we get a square
        m_background.SetGradientEdges(bounds.GetLow(), tCoord(bounds.GetLow().x + max, bounds.GetLow().y + max));
        m_background.BeginDraw();
        glBegin(GL_POLYGON);
        for(std::vector<tCoord>::iterator iter = se_rimWallRubberBand.begin(); iter != se_rimWallRubberBand.end(); ++iter) {
            m_background.DrawPoint(*iter);
        }
        m_background.DrawPoint(se_rimWallRubberBand.front());
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, .5);
    glBegin(GL_LINES);
    {
        for (int i=list.Len()-1; i >= 0; --i)
        {
            eWallRim *wall = list[i];
            eCoord begin = wall->EndPoint(0), end = wall->EndPoint(1);
            glVertex2f(begin.x, begin.y);
            glVertex2f(end.x, end.y);
        }


    }
    glEnd();
}

void Map::DrawWalls(tList<gNetPlayerWall> &list) {
    unsigned i, len=list.Len();
    double currentTime = se_GameTime();
    bool limitedLength = gCycle::WallsLength() > 0;
    double wallsStayUpDelay = gCycle::WallsStayUpDelay();
    glBegin(GL_LINES);
    for(i=0; i<len; i++) {
        gNetPlayerWall *wall = list[i];
        gCycle *cycle = wall->Cycle();
        if(!cycle) continue;
        double wallsLength = cycle->ThisWallsLength();
        double alpha = 1;
        if(!cycle->Alive() && wallsStayUpDelay >= 0) {
            alpha -= 2 * (currentTime - cycle->DeathTime() - wallsStayUpDelay);
            if(alpha <= 0) continue;
        }
        glColor4f(cycle->color_.r, cycle->color_.g, cycle->color_.b, alpha);
        double cycleDist = cycle->GetDistance();
        double minDist = limitedLength && cycleDist > wallsLength ? cycleDist - wallsLength : 0;
        const eCoord &begPos = wall->EndPoint(0), &endPos = wall->EndPoint(1);
        tArray<gPlayerWallCoord> &coords = wall->Coords();
        double begDist = wall->BegPos();
        double lenDist = wall->EndPos() - begDist;
        unsigned j, numcoords = coords.Len();
        if(numcoords < 2) continue;
        bool prevDangerous = coords[0].IsDangerous;
        double prevDist = coords[0].Pos;
        if(prevDist < minDist) prevDist = minDist;
        prevDist = (prevDist - begDist) / lenDist;
        for(j=1; j<numcoords; j++) {
            bool curDangerous = coords[j].IsDangerous;
            double curDist = coords[j].Pos;
            if(curDist < minDist) curDist = minDist;
            curDist = (curDist - begDist) / lenDist;
            if(prevDangerous) {
                glVertex2f(begPos.x + prevDist * (endPos.x - begPos.x), begPos.y + prevDist * (endPos.y - begPos.y));
                glVertex2f(begPos.x +  curDist * (endPos.x - begPos.x), begPos.y +  curDist * (endPos.y - begPos.y));
            }
            prevDangerous = curDangerous;
            prevDist = curDist;
        }
    }
    glEnd();
}

void Map::DrawObjects(tCoord scale) {
    tList<eGameObject> const &gameObjects = eGrid::CurrentGrid()->GameObjects();
    size_t len = gameObjects.Len();
    for(size_t i = 0; i < len; ++i) {
        eGameObject const *obj = gameObjects(i);
        tASSERT(obj);
        obj->Render2D(scale);
    }
}

void Map::ToggleMode(void) {
    if(m_modes.empty()) return;
    m_currentMode = (m_currentMode+1) % m_modes.size();
    Apply(m_modes[m_currentMode]);
}

Map::Map():
        m_mode(MODE_STD),
        m_zoom(3),
        m_rotation(ROTATION_SPAWN),
        m_toggleKey(0),
        m_clipper(new ClipperRect()),
        m_currentMode(0)
{
}

}
#endif
