#include "Morphose.h"
#include <ArduinoLog.h>
#include "WiFi.h"
#include "communications/osc.h"
#include <VectorXf.h>
#include <PlaquetteLib.h>

#define AVG_POSITION_TIME_WINDOW 0.2f

namespace morphose{

    int id;
    char name[16];
    int outgoingPort;

    Vec2f currPosition;

//TODO : MOve to morphose
    pq::Smoother avgPositionX(AVG_POSITION_TIME_WINDOW);
    pq::Smoother avgPositionY(AVG_POSITION_TIME_WINDOW);
    Vec2f avgPosition;

    void initialize(IPAddress ip)
    {   
        setID(ip[3]);
        setName(id);
        setOutgoingPort(id);

        Log.warningln("Morphose successfully initialized");

    }

    void setID(const int byte){
        id = (byte % 100) / 10;
        Log.infoln("Robot id is set to : %d", id);
    }

    void setName(const int id){
        sprintf(name, "robot%d", id);
        Log.infoln("Robot name is : %s", name);
    }

    void setOutgoingPort(const int id){
        outgoingPort = 8100 + (id*10);
        Log.infoln("Robot streaming port is : %d", outgoingPort);
    }

    void sayHello(){
        osc::send("/bonjour",name);
    }


    Vec2f getPosition()
    {
    return avgPosition;
    }

    void setCurrentPosition( Vec2f newPosition){
        currPosition.set(newPosition);
        Log.infoln("New position - x : %F y: %F", newPosition.x, newPosition.y);
    }

    void updateLocation()
    {
    // Update average positioning.
    avgPositionX.put( currPosition.x );
    avgPositionY.put( currPosition.y );
    avgPosition.set( avgPositionX.get(), avgPositionY.get() );
    }


    void update(){
        updateLocation();
    }
}//namespace Morphose