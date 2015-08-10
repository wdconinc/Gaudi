#ifndef GAUDIALG_TUPLEPUT_H
#define GAUDIALG_TUPLEPUT_H 1
// =============================================================================
// Include files
// =============================================================================
// GaudiKernel
// =============================================================================
#include "GaudiKernel/System.h"
// =============================================================================
// GaudiAlg
// =============================================================================
#include "GaudiAlg/TupleObj.h"
// ============================================================================
// ROOT TClass
// ============================================================================
#include "TClass.h"
// =============================================================================
/** @file
 *  Implementation file for Tuple::TupleObj::put method
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-04-08
 */
// =============================================================================
namespace Tuples
{
  /** @class ItemStore TuplePut.h GaudiAlg/TuplePut.h
   *
   *  Simple class, which represents the local storage of N-tuple items
   *  of the given type. Essentially it is a restricted
   *  GaudiUtils::HashMap with the ownership of the newly created entries
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-04-08
   */
  template <class VALUE>
  class ItemStore
  {
    friend class TupleObj ;
  private:
    typedef GaudiUtils::HashMap<std::string,NTuple::Item<VALUE>*>  Store;
  public:
    /// constructor : create empty map
    ItemStore()  = default;
    /// destructor : delete all known entries
    ~ItemStore()
    {
      for ( auto & i : m_map ) { delete i.second ; }
    }
  protected:
    /// the only one method:
    inline NTuple::Item<VALUE>* getItem
    ( const std::string& key , Tuples::TupleObj* tuple )
    {
      // find the item by name
      auto ifound = m_map.find( key ) ;
      // existing item?
      if ( m_map.end() != ifound ) { return ifound->second ; }        // RETURN
      // check the tuple for booking:
      if ( !tuple ) { return nullptr ; }
      // check the existence of the name
      if ( !tuple->goodItem ( key ) )
      {
        tuple -> Error ( "ItemStore::getItem('" + key
                         + "') item name is not unique").ignore() ;
        return nullptr ;                                                    // RETURN
      }
      // get the underlying object
      NTuple::Tuple* tup = tuple->tuple() ;
      if ( !tup )
      {
        tuple -> Error ( "ItemStore::getItem('" + key
                         + "') invalid NTuple::Tuple*" ).ignore() ;
        return nullptr ;                                                   // RETURN
      }
      // create new item:
      NTuple::Item<VALUE>* item = new NTuple::Item<VALUE>() ;
      // add it into N-tuple
      StatusCode sc = tup->addItem( key , *item ) ;                 // ATTENTION!
      if ( sc.isFailure() )
      {
        tuple -> Error   ( "ItemStore::getItem('" + key
                           + "') cannot addItem" , sc ).ignore() ;
        return nullptr ;                                                  // RETURN
      }
      // check the name again
      if ( !tuple->addItem( key , System::typeinfoName ( typeid ( VALUE ) ) ) )
      {
        tuple -> Warning ( "ItemStore::getItem('" + key
                           + "') the item not unique " ).ignore() ;
      }
      // add the newly created item into the store:
      if ( !m_map.insert ( std::make_pair ( key , item ) ).second )
      {
        tuple -> Warning ( "ItemStore::getItem('" + key
                           + "') item is not inserted!" ).ignore() ;
      }
      //
      return item ;                                                  // RETURN
    }
  private:
    // delete copy constructor and assignment
    ItemStore           ( const ItemStore& ) = delete;
    ItemStore& operator=( const ItemStore& ) = delete;
  private:
    /// the underlying map
    Store m_map ; ///< the underlying map
  } ;
} // end of namespace Tuples
// =============================================================================
/** The function allows to add almost arbitrary object into N-tuple
 *  @attention it requires POOL persistency
 *
 *  @param name column name
 *  @param obj  pointer to the object
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-04-08
 */
// =============================================================================
template <class TYPE>
inline StatusCode Tuples::TupleObj::put
( const std::string& name , const TYPE* obj )
{
  if (  invalid    () ) { return InvalidTuple     ; }   // RETURN
  if ( !evtColType () ) { return InvalidOperation ; }   // RETURN

  // static block: The type description & the flag
  static bool               s_fail = false ;                // STATIC
  static TClass*            s_type = 0     ;                // STATIC
  // check the status
  if      (  s_fail  ) { return InvalidItem ; }                           // RETURN
  else if ( !s_type  )
  {
    s_type = TClass::GetClass(typeid(TYPE));
    if ( !s_type )
    {
      s_fail = true ;
      return Error ( " put('"+name+"'," + System::typeinfoName(typeid(TYPE)) +
                     ") :Invalid ROOT Type", InvalidItem ) ;    // RETURN
    }
  }
  // the local storage of items
  static Tuples::ItemStore<TYPE*> s_map ;
  // get the variable by name:
  NTuple::Item<TYPE*>* item = s_map.getItem ( name , this ) ;
  if ( 0 == item )
  { return Error ( " put('" + name + "'): invalid item detected", InvalidItem ) ; }
  // assign the item!
  (*item) = const_cast<TYPE*> ( obj ) ;                    // THATS ALL!!
  //
  return StatusCode::SUCCESS ;                             // RETURN
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_TUPLEPUT_H
// ============================================================================
