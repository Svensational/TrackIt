#ifndef IDCOUNTER_H
#define IDCOUNTER_H

/// A simple define to make calling the global instance easier @relates IDCounter
#define idCounter IDCounter::getGlobalInstance()

/// Class managing the unique object IDs
/** The class has a global instance and everytime you request a new ID the
  * counter gets increased. You can also reset the counter or set it to a given
  * ID.
  */
class IDCounter {

public:
   /// Default c'tor.
   IDCounter();
   /// Returns the next free ID.
   int getID();
   /// Resets the counter to the given \a ID.
   void reset(int id = 0);
   /// returns a pointer to the global instance
   static IDCounter * getGlobalInstance();

private:
   int nextID; ///< the next ID that can be requested
};

#endif // IDCOUNTER_H
