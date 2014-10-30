#ifndef EFGRAPHVISITORS_H_
#define EFGRAPHVISITORS_H_

#include "IGraphVisitor.h"
#include "ExecutionFlowGraph.h"

namespace concurrency {

  class RunSimulator : public IGraphVisitor {
  public:
    /// Constructor
    RunSimulator(const int& slotNum) {
      m_nodesSucceeded = 0;
      m_slotNum = slotNum;
    };
    /// Destructor
    virtual ~RunSimulator() {};

    virtual bool visitEnter(DecisionNode& node) const;

    virtual bool visit(DecisionNode& node);

    virtual bool visitLeave(DecisionNode& node) const;


    virtual bool visitEnter(AlgorithmNode& node) const;

    virtual bool visit(AlgorithmNode& node);


    virtual void reset() { m_nodesSucceeded = 0; }

  };

  /** A visitor, performing full top-down traversals of a graph
   *
   */
  class Trigger : public IGraphVisitor {
    public:
      /// Constructor
      Trigger(const int& slotNum) {
        m_nodesSucceeded = 0;
        m_slotNum = slotNum;
      };
      /// Destructor
      virtual ~Trigger() {};

      virtual bool visitEnter(DecisionNode& node) const;

      virtual bool visit(DecisionNode& node);

      virtual bool visitLeave(DecisionNode& node) const;


      virtual bool visitEnter(AlgorithmNode& node) const;

      virtual bool visit(AlgorithmNode& node);


      virtual void reset() { m_nodesSucceeded = 0; }

    };

  class RankerByProductConsumption : public IGraphVisitor {
    public:
      /// Constructor
      RankerByProductConsumption() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      virtual ~RankerByProductConsumption() {};

      virtual bool visitEnter(DecisionNode&) const {return true;};

      virtual bool visit(DecisionNode&) {return true;};

      virtual bool visitLeave(DecisionNode&) const {return true;};


      virtual bool visitEnter(AlgorithmNode&) const {return true;};

      virtual bool visit(AlgorithmNode& node);


      virtual void reset() { m_nodesSucceeded = 0; }

      };

  class RankerByExecutionBranchPotential : public IGraphVisitor {
    public:
      /// Constructor
      RankerByExecutionBranchPotential() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      virtual ~RankerByExecutionBranchPotential() {};

      virtual bool visitEnter(DecisionNode&) const {return true;};

      virtual bool visit(DecisionNode&) {return true;};

      virtual bool visitLeave(DecisionNode&) const {return true;};


      virtual bool visitEnter(AlgorithmNode&) const {return true;};

      virtual bool visit(AlgorithmNode& node);


      virtual void reset() { m_nodesSucceeded = 0; }

      void runThroughAdjacents(boost::graph_traits<boost::ExecPlan>::vertex_descriptor vertex, boost::ExecPlan graph);

      };

  class RankerByTiming : public IGraphVisitor {
    public:
      /// Constructor
      RankerByTiming() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      virtual ~RankerByTiming() {};

      virtual bool visitEnter(DecisionNode&) const {return true;};

      virtual bool visit(DecisionNode&) {return true;};

      virtual bool visitLeave(DecisionNode&) const {return true;};


      virtual bool visitEnter(AlgorithmNode&) const {return true;};

      virtual bool visit(AlgorithmNode& node);


      virtual void reset() { m_nodesSucceeded = 0; }

      };

  class RankerByEccentricity : public IGraphVisitor {
    public:
      /// Constructor
      RankerByEccentricity() {
        m_nodesSucceeded = 0;
        m_slotNum = -1;
      };
      /// Destructor
      virtual ~RankerByEccentricity() {};

      virtual bool visitEnter(DecisionNode&) const {return true;};

      virtual bool visit(DecisionNode&) {return true;};

      virtual bool visitLeave(DecisionNode&) const {return true;};


      virtual bool visitEnter(AlgorithmNode&) const {return true;};

      virtual bool visit(AlgorithmNode& node);


      virtual void reset() { m_nodesSucceeded = 0; }

      };

}



#endif /* EFGRAPHVISITORS_H_ */
