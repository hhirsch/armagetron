#include "zShape.hpp"
#include "gCycle.h"
#include "zZone.h"

zShape::zShape(eGrid* grid, unsigned short idZone)
        :eNetGameObject( grid, eCoord(0,0), eCoord(0,0), NULL, true ),
        posx_(),
        posy_(),
        scale_(),
        rotation_(),
        color_(),
#ifdef DADA
        posxExpr(),
        posyExpr(),
        scaleExpr(),
        rotationExpr(),
#endif
        createdtime_(0.0),
        referencetime_(0.0),
        lasttime_(0.0),
        idZone_(idZone),
        newIdZone_(false)
{
    joinWithZone();
}

zShape::zShape(nMessage &m):eNetGameObject(m)
{
    REAL time;
    m >> time;
    setCreatedTime(time);

    networkRead(m);

    unsigned short anIdZone;
    m >> anIdZone;
    if(anIdZone != idZone_) {
        idZone_ = anIdZone;
        newIdZone_ = true;
        joinWithZone();
    }

}


void zShape::setCreatedTime(REAL time)
{
    createdtime_ = time;

    // Usefull when the nMessage creates the zone in the middle of a round
    if(lasttime_ < createdtime_)
        lasttime_ = createdtime_;
}

void zShape::setReferenceTime(REAL time)
{
    referencetime_ = time;
    // Do not update lasttime_, referencetime_ might be set in the future for ease of equation writing.
}

void zShape::networkWrite(nMessage &m)
{

    m << referencetime_;
#ifdef DADA
    m << posxExpr;
    m << posyExpr;
    m << scaleExpr;
    m << rotationExpr;
#else
    m << posx_;
    m << posy_;
    m << scale_;
    m << rotation_;
#endif
    m << color_.r_;
    m << color_.g_;
    m << color_.b_;
    m << color_.a_;
}

void zShape::networkRead(nMessage &m)
{
    REAL time;
    m >> time;
    setReferenceTime(time);
#ifdef DADA
    tString str;
    m >> str;
    setPosX(BasePtr(new tValue::Expr(str, tValue::Expr::vars, tValue::Expr::functions)) , str);

    m >> str;
    setPosY(BasePtr(new tValue::Expr(str, tValue::Expr::vars, tValue::Expr::functions)) , str);

    m >> str;
    setScale(BasePtr(new tValue::Expr(str, tValue::Expr::vars, tValue::Expr::functions)) , str);

    m >> str;
    setRotation(BasePtr(new tValue::Expr(str, tValue::Expr::vars, tValue::Expr::functions)) , str);
#else
    m >> posx_;
    m >> posy_;
    m >> scale_;
    m >> rotation_;
#endif

    m >> color_.r_;
    m >> color_.g_;
    m >> color_.b_;
    m >> color_.a_;
}

/*
 * to create a shape on the clients
 */
void zShape::WriteCreate( nMessage & m )
{
    eNetGameObject::WriteCreate(m);

    m << createdtime_;

    networkWrite(m);

    m << idZone_;
}

void zShape::WriteSync(nMessage &m)
{
    networkWrite(m);
}

void zShape::ReadSync(nMessage &m)
{
    networkRead(m);
}

#ifdef DADA
void zShape::setPosX(const BasePtr & x, tString &exprStr){
    posx_ = x;
    posxExpr = exprStr;
}

void zShape::setPosY(const BasePtr & y, tString &exprStr){
    posy_ = y;
    posyExpr = exprStr;
}

void zShape::setRotation(const BasePtr & r, tString &exprStr){
    rotation_ = r;
    rotationExpr = exprStr;
}

void zShape::setScale(const BasePtr & s, tString &exprStr){
    scale_ = s;
    scaleExpr = exprStr;
}
#else
void zShape::setPosX(const tFunction & x){
    posx_ = x;
}

void zShape::setPosY(const tFunction & y){
    posy_ = y;
}

void zShape::setRotation(const tFunction & r){
    rotation_ = r;
    if (sn_GetNetState()!=nCLIENT)
        RequestSync();
}

void zShape::setRotationNow(const tFunction & r){
    float pos = rotation_.Evaluate(lasttime_ - referencetime_);
    rotation_.SetSlope(r.GetSlope() );
    rotation_.SetOffset( pos + rotation_.GetSlope() * ( referencetime_ - lasttime_ ) );
    //    rotation_ = r;
    if (sn_GetNetState()!=nCLIENT)
        RequestSync();
}

void zShape::setScale(const tFunction & s){
    scale_ = s;
}
#endif

void zShape::setColor(const rColor &c){
    if(color_ != c) {
        color_ = c;
        if (sn_GetNetState()!=nCLIENT)
            RequestSync();
    }
}

void zShape::setColorNow(const rColor &c){
    // TODO: make color slide with some fancy tFunc or something.
    setColor(c);
}

void zShape::animate( REAL time ) {
    // Is this needed as the items are already animated?
}

void zShape::TimeStep( REAL time ) {
    lasttime_ = time;
    /*
    REAL scale = scale_.Evaluate(lasttime_ - referencetime_);
    if (scale < -1.0) // Allow shapes to keep existing for a while even though they are not physical
      {
        // The shape has collapsed and should be removed
      }
    */

    if(newIdZone_) {
        joinWithZone();
    }

}

bool zShape::isInteracting(eGameObject * target) {
    return false;
}

void zShape::render(const eCamera *cam )
{}
void zShape::render2d(tCoord scale) const
    {}

void zShape::joinWithZone() {
    if(sn_netObjects[idZone_]) {
        zZone *asdf = dynamic_cast<zZone*>(&*sn_netObjects[idZone_]);
        asdf->setShape(zShapePtr(this));
        newIdZone_ = false;
    }
}

zShapeCircle::zShapeCircle(eGrid *grid, unsigned short idZone):
        zShape(grid, idZone),
        emulatingOldZone_(false),
        radius(1.0, 0.0)
{}

zShapeCircle::zShapeCircle(nMessage &m):
        zShape(m),
        emulatingOldZone_(false),
        radius(1.0, 0.0)
{
    m >> radius;
}

/*
 * to create a shape on the clients
 */
void zShapeCircle::WriteCreate( nMessage & m )
{
    zShape::WriteCreate(m);

    m << radius;
}

void zShapeCircle::WriteSync(nMessage &m)
{
    zShape::WriteSync(m);
    m << radius;
}

void zShapeCircle::ReadSync(nMessage &m)
{
    zShape::ReadSync(m);
    m >> radius;
}

bool zShapeCircle::isInteracting(eGameObject * target)
{
    bool interact = false;
    gCycle* prey = dynamic_cast< gCycle* >( target );
    if ( prey )
    {
        if ( prey->Player() && prey->Alive() )
        {
            REAL effectiveRadius;
#ifdef DADA
            effectiveRadius = scale_->GetFloat()  * radius.Evaluate(lasttime_ - referencetime_);
#else
            effectiveRadius = scale_.Evaluate(lasttime_ - referencetime_) * radius.Evaluate(lasttime_ - referencetime_);
#endif
            // Is the player inside or outside the zone
            if ( (effectiveRadius >= 0.0) && ( prey->Position() - Position() ).NormSquared() < effectiveRadius*effectiveRadius )
            {
                interact = true;
            }
        }
    }
    return interact;
}

void zShapeCircle::render(const eCamera * cam )
{
#ifndef DEDICATED

    // HACK
    int sg_segments = 11;
    bool sr_alphaBlend = true;
    // HACK

    if ( color_.a_ > .7f )
        color_.a_ = .7f;
    if ( color_.a_ <= 0 )
        return;

#ifdef DADA
    eCoord rot(cos(rotation_->GetFloat()) , sin(rotation_->GetFloat()));
#else
    //    eCoord rot(cos(rotation_.Evaluate(lasttime_ - referencetime_) ), sin(rotation_.Evaluate(lasttime_ - referencetime_)));
    eCoord rot(1,0);
    REAL currAngle = rotation_.Evaluate(lasttime_ - referencetime_);
    rot = rot.Turn( cos(currAngle), sin(currAngle) );
#endif

    GLfloat m[4][4]={{rot.x,rot.y,0,0},
                     {-rot.y,rot.x,0,0},
                     {0,0,1,0},
#ifdef DADA
                     {posx_->GetFloat(),posy_->GetFloat(),0,1}};
#else
                     {posx_.Evaluate(lasttime_ - referencetime_), posy_.Evaluate(lasttime_ - referencetime_), 0,1}};
#endif

    ModelMatrix();
    glPushMatrix();

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    //glDisable(GL_TEXTURE);
    glDisable(GL_TEXTURE_2D);

    //	glTranslatef(pos.x,pos.y,0);

    glMultMatrixf(&m[0][0]);
    //	glScalef(.5,.5,.5);

    if ( sr_alphaBlend )
        BeginQuads();
    else
        BeginLineStrip();

    const REAL seglen = .2f;
    const REAL bot = 0.0f;
    const REAL top = 5.0f; // + ( lastTime - referenceTime_ ) * .1f;

    color_.Apply();

    REAL effectiveRadius;
#ifdef DADA
    effectiveRadius = scale_->GetFloat()  * radius.Evaluate(lasttime_ - referencetime_);
#else
    effectiveRadius = scale_.Evaluate(lasttime_ - referencetime_) * radius.Evaluate(lasttime_ - referencetime_);
#endif
    if (effectiveRadius >= 0.0)
    {
        for ( int i = sg_segments - 1; i>=0; --i )
        {
            REAL a = i * 2 * 3.14159 / REAL( sg_segments );
            REAL b = a + seglen;

            REAL sa = effectiveRadius * sin(a);
            REAL ca = effectiveRadius * cos(a);
            REAL sb = effectiveRadius * sin(b);
            REAL cb = effectiveRadius * cos(b);

            glVertex3f(sa, ca, bot);
            glVertex3f(sa, ca, top);
            glVertex3f(sb, cb, top);
            glVertex3f(sb, cb, bot);

            if ( !sr_alphaBlend )
            {
                glVertex3f(sa, ca, bot);
                RenderEnd();
                BeginLineStrip();
            }
        }
    }

    RenderEnd();

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDepthMask(GL_TRUE);

    glPopMatrix();
#endif

}

//HACK: render2d and render should probably be merged somehow, too much copy and paste here

void zShapeCircle::render2d(tCoord scale) const {
#ifndef DEDICATED

    // HACK
    int sg_segments = 8;
    // HACK

    //if ( color_.a_ > .7f )
    //    color_.a_ = .7f;
    if ( color_.a_ <= 0 )
        return;

#ifdef DADA
    eCoord rot(cos(rotation_->GetFloat()) , sin(rotation_->GetFloat()));
#else
    //    eCoord rot(cos(rotation_.Evaluate(lasttime_ - referencetime_) ), sin(rotation_.Evaluate(lasttime_ - referencetime_)));
    eCoord rot(1,0);
    REAL currAngle = rotation_.Evaluate(lasttime_ - referencetime_);
    rot = rot.Turn( cos(currAngle), sin(currAngle) );
#endif

    GLfloat m[4][4]={{rot.x,rot.y,0,0},
                     {-rot.y,rot.x,0,0},
                     {0,0,1,0},
#ifdef DADA
                     {posx_->GetFloat(),posy_->GetFloat(),0,1}};
#else
                     {posx_.Evaluate(lasttime_ - referencetime_), posy_.Evaluate(lasttime_ - referencetime_), 0,1}};
#endif

    ModelMatrix();
    glPushMatrix();

    glMultMatrixf(&m[0][0]);

    BeginLines();

    const REAL seglen = M_PI / sg_segments;

    color_.Apply();

    REAL effectiveRadius;
#ifdef DADA
    effectiveRadius = scale_->GetFloat()  * radius.Evaluate(lasttime_ - referencetime_);
#else
    effectiveRadius = scale_.Evaluate(lasttime_ - referencetime_) * radius.Evaluate(lasttime_ - referencetime_);
#endif
    if (effectiveRadius >= 0.0)
    {
        for ( int i = sg_segments - 1; i>=0; --i )
        {
            REAL a = i * 2 * 3.14159 / REAL( sg_segments );
            REAL b = a + seglen;

            REAL sa = effectiveRadius * sin(a);
            REAL ca = effectiveRadius * cos(a);
            REAL sb = effectiveRadius * sin(b);
            REAL cb = effectiveRadius * cos(b);

            glVertex2f(sa, ca);
            glVertex2f(sb, cb);
        }
    }
    RenderEnd();
    glPopMatrix();
#endif
}

//
zShapePolygon::zShapePolygon(nMessage &m):zShape(m),
#ifdef DADA
        exprs(),
#endif
        points()
{
    int numPoints;
    m >> numPoints;

    // read the polygon shape
    for( ; numPoints>0 && !m.End(); numPoints-- )
    {
#ifdef DADA
        tString strX, strY;
        m >> strX;
        m >> strY;

        tValue::BasePtr xp = tValue::BasePtr( new tValue::Expr (strX, tValue::Expr::vars, tValue::Expr::functions) );
        tValue::BasePtr yp = tValue::BasePtr( new tValue::Expr (strY, tValue::Expr::vars, tValue::Expr::functions) );

        addPoint( myPoint( xp, yp ), std::pair<tString, tString>(strX, strY) );
#else
        tFunction tfX, tfY;
        m >> tfX;
        m >> tfY;

        addPoint( myPoint( tfX, tfY ) );
#endif
    }
}

zShapePolygon::zShapePolygon(eGrid *grid, unsigned short idZone):
        zShape(grid, idZone),
#ifdef DADA
        exprs(),
#endif
        points()
{}

/*
 * to create a shape on the clients
 */
void zShapePolygon::WriteCreate( nMessage & m )
{
    zShape::WriteCreate(m);

    int numPoints;
    numPoints = points.size();
    m << numPoints;

#ifdef DADA
    std::vector< std::pair<tString, tString> >::const_iterator iter;
    for(iter = exprs.begin();
            iter != exprs.end();
            ++iter)
    {
        m << (*iter).first;
        m << (*iter).second;
    }
#else
    std::vector< myPoint >::const_iterator iter;
    for(iter = points.begin();
            iter != points.end();
            ++iter)
    {
        m << (*iter).first;
        m << (*iter).second;
    }
#endif

    //    WriteSync( m );
}

bool zShapePolygon::isInside(eCoord anECoord) {
    // The following is a modified version of code by Randolph Franklin,
    // Found at http://local.wasp.uwa.edu.au/~pbourke/geometry/insidepoly/

    // Hack!!!!
    // All the poinst need to be moved around the x,y position
    // then scaled
    //    REAL r = scale_->GetFloat();
    REAL x = anECoord.x;
    REAL y = anECoord.y;
    int c = 0;

    std::vector< myPoint >::const_iterator iter = points.end() - 1;
    // We need to position the prevIter to the last point.
    REAL currentScale = 0.0;
#ifdef DADA
    REAL x_ = (*iter).first->GetFloat();
    REAL y_ = (*iter).second->GetFloat();
    tCoord centerPos = tCoord(posx_->GetFloat(), posy_->GetFloat());
    tCoord rotation = tCoord( cosf(rotation_->GetFloat()), sinf(rotation_->GetFloat()) );
    currentScale = scale_->GetFloat();
    tCoord previous = tCoord(x_, y_).Turn( rotation )*currentScale + centerPos;
#else
    REAL x_ = (*iter).first.Evaluate(lasttime_ - referencetime_);
    REAL y_ = (*iter).second.Evaluate(lasttime_ - referencetime_);
    tCoord centerPos = tCoord(posx_.Evaluate(lasttime_ - referencetime_), posy_.Evaluate(lasttime_ - referencetime_));
    tCoord rotation = tCoord( cosf(rotation_.Evaluate(lasttime_ - referencetime_)), sinf(rotation_.Evaluate(lasttime_ - referencetime_)) );
    currentScale = scale_.Evaluate(lasttime_ - referencetime_);
    tCoord previous = tCoord(x_, y_).Turn( rotation )*currentScale + centerPos;
#endif
    REAL xpp = previous.x;
    REAL ypp = previous.y;

    if(currentScale > 0.0)
    {
        for(iter = points.begin();
                iter != points.end();
                ++iter)
        {
#ifdef DADA
            x_ = (*iter).first->GetFloat();
            y_ = (*iter).second->GetFloat();
            tCoord current = tCoord(x_, y_).Turn( rotation )*currentScale + centerPos;
#else
            x_ = (*iter).first.Evaluate(lasttime_ - referencetime_);
            y_ = (*iter).second.Evaluate(lasttime_ - referencetime_);
            tCoord current = tCoord(x_, y_).Turn( rotation )*currentScale + centerPos;
#endif
            REAL xp = current.x;
            REAL yp = current.y;

            if ((((yp <= y) && (y < ypp)) || ((ypp <= y) && (y < yp))) &&
                    (x < (xpp - xp) * (y - yp) / (ypp - yp) + xp))
                c = !c;

            xpp = xp; ypp = yp;
        }
    }

    return c;
}
#include "eNetGameObject.h"
#include "ePlayer.h"
bool zShapePolygon::isInteracting(eGameObject * target)
{
    bool interact = false;
    gCycle* prey = dynamic_cast< gCycle* >( target );
    if ( prey )
    {
        if ( prey->Player() && prey->Alive() )
        {
            // Is the player inside or outside the zone
            if ( isInside( prey->Position() ) )
            {
                interact = true;
            }
        }
    }
    return interact;
}

void zShapePolygon::render(const eCamera * cam )
{
#ifndef DEDICATED

    // HACK
    //  int sg_segments = 11;
    bool sr_alphaBlend = true;
    // HACK

    if ( color_.a_ > .7f )
        color_.a_ = .7f;
    if ( color_.a_ <= 0 )
        return;


    ModelMatrix();
    glPushMatrix();

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    //glDisable(GL_TEXTURE);
    glDisable(GL_TEXTURE_2D);

    //    glMultMatrixf(&m[0][0]);

    REAL currentScale = 0.0;
#ifdef DADA
    glTranslatef(posx_->GetFloat(), posy_->GetFloat(), 0);

    currentScale = scale_->GetFloat();
#else
    glTranslatef(posx_.Evaluate(lasttime_ - referencetime_), posy_.Evaluate(lasttime_ - referencetime_), 0);

    currentScale = scale_.Evaluate(lasttime_ - referencetime_);
#endif

    if(currentScale > 0.0)
    {
        glScalef(currentScale, currentScale, 1.0);

#ifdef DADA
        glRotatef(rotation_->GetFloat()*180/M_PI, 0.0, 0.0, 1.0);
#else
        glRotatef(rotation_.Evaluate(lasttime_ - referencetime_)*180/M_PI, 0.0, 0.0, 1.0);
#endif

        if ( sr_alphaBlend )
            BeginQuads();
        else
            BeginLineStrip();

        //    const REAL seglen = .2f;
        const REAL bot = 0.0f;
        const REAL top = 5.0f; // + ( lastTime - createTime_ ) * .1f;

        color_.Apply();




        std::vector< myPoint >::const_iterator iter;
        std::vector< myPoint >::const_iterator prevIter = points.end() - 1;

        for(iter = points.begin();
                iter != points.end();
                prevIter = iter++)
        {
#ifdef DADA
            REAL xp = (*iter).first->GetFloat() ;
            REAL yp = (*iter).second->GetFloat() ;
            REAL xpp = (*prevIter).first->GetFloat() ;
            REAL ypp = (*prevIter).second->GetFloat() ;
#else
            REAL xp = (*iter).first.Evaluate( lasttime_ - referencetime_ ) ;
            REAL yp = (*iter).second.Evaluate( lasttime_ - referencetime_ ) ;
            REAL xpp = (*prevIter).first.Evaluate( lasttime_ - referencetime_ ) ;
            REAL ypp = (*prevIter).second.Evaluate( lasttime_ - referencetime_ ) ;
#endif

            glVertex3f(xp, yp, bot);
            glVertex3f(xp, yp, top);
            glVertex3f(xpp, ypp, top);
            glVertex3f(xpp, ypp, bot);

            if ( !sr_alphaBlend )
            {
                glVertex3f(xp, yp, bot);
                RenderEnd();
                BeginLineStrip();
            }
        }

        RenderEnd();

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glDepthMask(GL_TRUE);
    }
    glPopMatrix();
#endif
}
void zShapePolygon::render2d(tCoord scale) const {
#ifndef DEDICATED

    //if ( color_.a_ > .7f )
    //    color_.a_ = .7f;
    if ( color_.a_ <= 0 )
        return;


    ModelMatrix();
    glPushMatrix();

    REAL currentScale = 0.0;
#ifdef DADA
    glTranslatef(posx_->GetFloat(), posy_->GetFloat(), 0);

    currentScale = scale_->GetFloat();
#else
    glTranslatef(posx_.Evaluate(lasttime_ - referencetime_), posy_.Evaluate(lasttime_ - referencetime_), 0);

    currentScale = scale_.Evaluate(lasttime_ - referencetime_);
#endif

    if(currentScale > 0.0)
    {
        glScalef(currentScale, currentScale, 1.0);

#ifdef DADA
        glRotatef(rotation_->GetFloat()*180/M_PI, 0.0, 0.0, 1.0);
#else
        glRotatef(rotation_.Evaluate(lasttime_ - referencetime_)*180/M_PI, 0.0, 0.0, 1.0);
#endif

        BeginLines();

        //    const REAL seglen = .2f;

        color_.Apply();

        std::vector< myPoint >::const_iterator iter;
        std::vector< myPoint >::const_iterator prevIter = points.end() - 1;

        for(iter = points.begin();
                iter != points.end();
                prevIter = iter++)
        {
#ifdef DADA
            REAL xp = (*iter).first->GetFloat() ;
            REAL yp = (*iter).second->GetFloat() ;
            REAL xpp = (*prevIter).first->GetFloat() ;
            REAL ypp = (*prevIter).second->GetFloat() ;
#else
            REAL xp = (*iter).first.Evaluate( lasttime_ - referencetime_ ) ;
            REAL yp = (*iter).second.Evaluate( lasttime_ - referencetime_ ) ;
            REAL xpp = (*prevIter).first.Evaluate( lasttime_ - referencetime_ ) ;
            REAL ypp = (*prevIter).second.Evaluate( lasttime_ - referencetime_ ) ;
#endif

            glVertex2f(xp, yp);
            glVertex2f(xpp, ypp);
        }

        RenderEnd();
    }
    glPopMatrix();
#endif
}

// the shapes's network initializator
static nNOInitialisator<zShapeCircle> zoneCircle_init(350,"shapeCircle");
static nNOInitialisator<zShapePolygon> zonePolygon_init(360,"shapePolygon");

nDescriptor & zShapeCircle::CreatorDescriptor( void ) const
{
    return zoneCircle_init;
}

nDescriptor & zShapePolygon::CreatorDescriptor( void ) const
{
    return zonePolygon_init;
}