#ifndef GAUDISVC_EXCEPTIONSVC_H
#define GAUDISVC_EXCEPTIONSVC_H
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IExceptionSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/MsgStream.h"
// ============================================================================
/** @class ExceptionSvc
 *  Simple implementation of IExceptionSvc abstract interface
 *  @author (1) ATLAS collaboration
 *  @author (2) modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-08
 */
// ============================================================================
class ExceptionSvc: public extends1<Service, IExceptionSvc> {
public:
  /// Handle caught GaudiExceptions
  StatusCode handle
  ( const INamedInterface& o ,
    const GaudiException&  e ) const override; ///< Handle caught exceptions
  /// Handle caught std::exceptions
  StatusCode handle
  ( const INamedInterface& o ,
    const std::exception & e ) const override; ///< Handle caught exceptions
  /// Handle caught (unknown)exceptions
  StatusCode handle
  ( const INamedInterface& o ) const override; ///< Handle caught exceptions
  /// Handle errors
  StatusCode handleErr
  ( const INamedInterface& o ,
    const StatusCode&      s ) const override; ///< Handle errors
public:
  /// initialize the service
  StatusCode initialize   () override;
public:
  /** standard constructor
   *  @param name service instance name
   *  @param pSvc pointer to Service Locator
   */
  ExceptionSvc
  ( const std::string& name ,
    ISvcLocator*       svc  ) ;
  /// Destructor.
  ~ExceptionSvc() override = default;
private:
  // default constructor is disabled
  ExceptionSvc () = delete; ///< no default constructor
  // copy constructor is disabled
  ExceptionSvc ( const ExceptionSvc& ) = delete;
  // assignment operator is disabled
  ExceptionSvc& operator=( const ExceptionSvc& ) = delete;
  // process exceptions
  virtual StatusCode process  ( const INamedInterface& o ) const ;
private:

  enum Policy { ALL, NONE };
  enum ReturnState { SUCCESS, FAILURE, RECOVERABLE, RETHROW, DEFAULT };

  Policy m_mode_exc = ALL, m_mode_err = NONE;
  StringProperty m_mode_exc_s, m_mode_err_s;
  std::map<std::string,ReturnState> m_retCodesExc, m_retCodesErr;

  mutable MsgStream m_log;

};

// ============================================================================
#endif // GAUDISVC_EXCEPTIONSVC_H
// ============================================================================
// The END
// ============================================================================
