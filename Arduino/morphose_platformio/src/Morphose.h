#ifndef MORPHOSE_H
#define MORPHOSE_H

#include <VectorXf.h>

class IPAddress;


namespace morphose{

    extern int id;
    extern char name[16];
    extern int outgoingPort;

    

    void initialize(IPAddress ip);

    void update();

    void setID(const int byte);

    void setName(const int id);

    void setOutgoingPort(const int id);
    
    void sayHello();

    Vec2f getPosition() ;

    void setCurrentPosition( Vec2f newPosition);

}//namespace Morphose






#endif

