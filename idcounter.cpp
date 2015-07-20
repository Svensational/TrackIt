#include "idcounter.h"

/** The \ref nextID gets initialized to 0.
  */
IDCounter::IDCounter() :
   nextID(0)
{
}

/** The global instance is not really global but a function static one. Bazinga!
  */
IDCounter * IDCounter::getGlobalInstance() {
   static IDCounter * counter = new IDCounter();
   return counter;
}

int IDCounter::getID() {
   return nextID++;
}

/** If no ID is given it defaults to 0.
  */
void IDCounter::reset(int id) {
   nextID = id;
}
