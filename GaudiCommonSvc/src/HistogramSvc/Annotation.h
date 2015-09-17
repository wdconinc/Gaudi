/* Emacs: -*- C++ -*- */
#ifndef GAUDISVC_ANNOTATION_H
#define GAUDISVC_ANNOTATION_H 1
/// @FIXME: AIDA interfaces visibility
#include "AIDA_visibility_hack.h"

// Include files
#include <vector>
#include <string>
#include <map>

#include "AIDA/IAnnotation.h"

namespace AIDA {

///  Implementation of the AIDA IAnnotation interface class

class  Annotation  : virtual public IAnnotation {

public:
  /// Constructor
  Annotation() = default;

  /// Destructor
  ~Annotation() override = default;

  /// Add a key/value pair with a given sticky.
  bool addItem( const std::string & key,
                const std::string & value,
                bool sticky = false);

  /// Remove the item indicated by a given key
  bool removeItem( const std::string & key );

  /// Retrieve the value for a given key
  std::string value( const std::string & key) const;

  /// Set value for a given key
  void setValue( const std::string & key,
                 const std::string& value);

  /// Set sticky for a given key
  void setSticky( const std::string & key,
                  bool sticky);

  /// Get the number of items in the Annotation
  int size() const;

  /// Individual access to the Annotation-items
  std::string  key(int index) const;
  std::string  value(int index) const;

  /// Remove all the non-sticky items
  void reset();

private:
  /// Internal private annotation item class
  struct AnnotationItem final {
    AnnotationItem( std::string k = "", std::string v = "", bool vis = true):
      m_key( std::move(k) ), m_value( std::move(v) ), m_sticky( vis )
    {/* nop */};

    ~AnnotationItem() = default;

    std::string m_key;
    std::string m_value;
    bool        m_sticky;
  };

  /// The vector of the annotation items
  std::vector< AnnotationItem >         m_annotationItems;

  /// The map of strings to identifiers
  std::map< std::string, unsigned int > m_identifiers;

  std::string emptyString;
};
}

inline bool AIDA::Annotation::addItem( const std::string & key,
							  const std::string & value,
							  bool sticky )
{
  if ( m_identifiers.find( key ) != m_identifiers.end() ) return false;
  m_annotationItems.emplace_back( key, value, sticky );
  m_identifiers.emplace( key, m_annotationItems.size() - 1 );
  return true;
}

inline bool AIDA::Annotation::removeItem( const std::string & key )  {
  auto iKey = m_identifiers.find( key );
  if ( iKey == m_identifiers.end() ) return false;

  unsigned int indexToBeRemoved = iKey->second;
  // check stickness

  if (  m_annotationItems[indexToBeRemoved].m_sticky ) return false;

  // why rebuilding ?

  m_identifiers.clear();
  std::vector< AnnotationItem > annotationItemsNew;
  if ( m_annotationItems.size() > 1 ) annotationItemsNew.reserve( m_annotationItems.size() - 1 );
  for ( unsigned int iItem = 0; iItem < m_annotationItems.size(); ++iItem ) {
    if ( iItem == indexToBeRemoved ) continue;
    const auto& item = m_annotationItems[ iItem ];
    annotationItemsNew.emplace_back( item.m_key, item.m_value, item.m_sticky );
    m_identifiers.emplace( item.m_key, annotationItemsNew.size() - 1 );
  }
  m_annotationItems = std::move(annotationItemsNew);
  return true;
}

inline std::string AIDA::Annotation::value( const std::string & key) const
{
  auto iKey = m_identifiers.find( key );
  return iKey != m_identifiers.end() ? m_annotationItems[ iKey->second ].m_value 
                                     : emptyString;
}

inline void AIDA::Annotation::setValue( const std::string & key, const std::string& value)
{
  auto iKey = m_identifiers.find( key );
  if ( iKey == m_identifiers.end() )
    // if not found, then add it
    addItem(key,value);
  else
    m_annotationItems[ iKey->second ].m_value = value;
}

inline void AIDA::Annotation::setSticky( const std::string & key, bool sticky)
{
  auto iKey = m_identifiers.find( key );
  if ( iKey != m_identifiers.end() ) m_annotationItems[ iKey->second ].m_sticky = sticky;
}

inline int AIDA::Annotation::size() const  {
  return m_annotationItems.size();
}

inline std::string AIDA::Annotation::key(int index) const
{
  if ( index < 0 || index >= static_cast<int>(m_annotationItems.size()) ) return emptyString;
  return m_annotationItems[ index ].m_key;
}

inline std::string AIDA::Annotation::value(int index) const
{
  if ( index < 0 || index >= static_cast<int>(m_annotationItems.size()) ) return emptyString;
  return m_annotationItems[ index ].m_value;
}

inline void AIDA::Annotation::reset()
{
  // Collect the non-sticky items
  std::vector< std::string > itemsToRemove;
  itemsToRemove.reserve( size() );
  for ( const auto& item : m_annotationItems ) {
    if ( !item.m_sticky ) itemsToRemove.push_back( item.m_key );
  }
  for ( const auto& i : itemsToRemove ) removeItem(i);
}

#endif
