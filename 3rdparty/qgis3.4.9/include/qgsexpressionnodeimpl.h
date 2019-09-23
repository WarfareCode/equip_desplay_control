/***************************************************************************
                               qgsexpressionnodeimpl.h
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSEXPRESSIONNODEIMPL_H
#define QGSEXPRESSIONNODEIMPL_H

#include "qgsexpressionnode.h"
#include "qgsinterval.h"

/**
 * \ingroup core
 * A unary node is either negative as in boolean (not) or as in numbers (minus).
 */
class CORE_EXPORT QgsExpressionNodeUnaryOperator : public QgsExpressionNode
{
  public:

    /**
     * \brief list of unary operators
     * \note if any change is made here, the definition of QgsExpression::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * A node unary operator is modifying the value of \a operand by negating it with \a op.
     */
    QgsExpressionNodeUnaryOperator( QgsExpressionNodeUnaryOperator::UnaryOperator op, QgsExpressionNode *operand SIP_TRANSFER )
      : mOp( op )
      , mOperand( operand )
    {}
    ~QgsExpressionNodeUnaryOperator() override { delete mOperand; }

    /**
     * Returns the unary operator.
     */
    QgsExpressionNodeUnaryOperator::UnaryOperator op() const { return mOp; }

    /**
     * Returns the node the operator will operate upon.
     */
    QgsExpressionNode *operand() const { return mOperand; }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;

    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

    /**
     * Returns a the name of this operator without the operands.
     * I.e. "NOT" or "-"
     */
    QString text() const;

  private:
    UnaryOperator mOp;
    QgsExpressionNode *mOperand = nullptr;

    static const char *UNARY_OPERATOR_TEXT[];
};

/**
 * A binary expression operator, which operates on two values.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeBinaryOperator : public QgsExpressionNode
{
  public:

    /**
     * \brief list of binary operators
     * \note if any change is made here, the definition of QgsExpression::BinaryOperatorText[] must be adapted.
     */
    enum BinaryOperator
    {
      // logical
      boOr,
      boAnd,

      // comparison
      boEQ,  //!< =
      boNE,  //!< <>
      boLE,  //!< <=
      boGE,  //!< >=
      boLT,  //!< <
      boGT,  //!< >
      boRegexp,
      boLike,
      boNotLike,
      boILike,
      boNotILike,
      boIs,
      boIsNot,

      // math
      boPlus,
      boMinus,
      boMul,
      boDiv,
      boIntDiv,
      boMod,
      boPow,

      // strings
      boConcat,
    };

    /**
     * Binary combination of the left and the right with op.
     */
    QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::BinaryOperator op, QgsExpressionNode *opLeft SIP_TRANSFER, QgsExpressionNode *opRight SIP_TRANSFER )
      : mOp( op )
      , mOpLeft( opLeft )
      , mOpRight( opRight )
    {}
    ~QgsExpressionNodeBinaryOperator() override { delete mOpLeft; delete mOpRight; }

    /**
     * Returns the binary operator.
     */
    QgsExpressionNodeBinaryOperator::BinaryOperator op() const { return mOp; }

    /**
     * Returns the node to the left of the operator.
     * \see opRight()
     */
    QgsExpressionNode *opLeft() const { return mOpLeft; }

    /**
     * Returns the node to the right of the operator.
     * \see opLeft()
     */
    QgsExpressionNode *opRight() const { return mOpRight; }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes( ) const override; SIP_SKIP

    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

    /**
     * Returns the precedence index for the operator. Higher values have higher precedence.
     */
    int precedence() const;

    /**
     * Returns true if the operator is left-associative.
     */
    bool leftAssociative() const;

    /**
     * Returns a the name of this operator without the operands.
     * I.e. "AND", "OR", ...
     */
    QString text() const;

  private:
    bool compare( double diff );
    qlonglong computeInt( qlonglong x, qlonglong y );
    double computeDouble( double x, double y );

    /**
     * Computes the result date time calculation from a start datetime and an interval
     * \param d start datetime
     * \param i interval to add or subtract (depending on mOp)
     */
    QDateTime computeDateTimeFromInterval( const QDateTime &d, QgsInterval *i );

    BinaryOperator mOp;
    QgsExpressionNode *mOpLeft = nullptr;
    QgsExpressionNode *mOpRight = nullptr;

    static const char *BINARY_OPERATOR_TEXT[];
};

/**
 * An expression node for value IN or NOT IN clauses.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeInOperator : public QgsExpressionNode
{
  public:

    /**
     * This node tests if the result of \a node is in the result of \a list. Optionally it can be inverted with \a notin which by default is false.
     */
    QgsExpressionNodeInOperator( QgsExpressionNode *node SIP_TRANSFER, QgsExpressionNode::NodeList *list SIP_TRANSFER, bool notin = false )
      : mNode( node )
      , mList( list )
      , mNotIn( notin )
    {}
    ~QgsExpressionNodeInOperator() override;

    /**
     * Returns the expression node.
     */
    QgsExpressionNode *node() const { return mNode; }

    /**
     * Returns true if this node is a "NOT IN" operator, or false if the node is a normal "IN" operator.
     */
    bool isNotIn() const { return mNotIn; }

    /**
     * Returns the list of nodes to search for matching values within.
     */
    QgsExpressionNode::NodeList *list() const { return mList; }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:
    QgsExpressionNode *mNode = nullptr;
    QgsExpressionNodeInOperator::NodeList *mList = nullptr;
    bool mNotIn;
};

/**
 * An expression node for expression functions.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeFunction : public QgsExpressionNode
{
  public:

    /**
     * A function node consists of an index of the function in the global function array and
     * a list of arguments that will be passed to it.
     */
    QgsExpressionNodeFunction( int fnIndex, QgsExpressionNode::NodeList *args SIP_TRANSFER );

    ~QgsExpressionNodeFunction() override;

    /**
     * Returns the index of the node's function.
     */
    int fnIndex() const { return mFnIndex; }

    /**
     * Returns a list of arguments specified for the function.
     */
    QgsExpressionNode::NodeList *args() const { return mArgs; }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;

    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

    //! Tests whether the provided argument list is valid for the matching function
    static bool validateParams( int fnIndex, QgsExpressionNode::NodeList *args, QString &error );

  private:
    int mFnIndex;
    NodeList *mArgs = nullptr;
};

/**
 * An expression node for literal values.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeLiteral : public QgsExpressionNode
{
  public:

    /**
     * Constructor for QgsExpressionNodeLiteral, with the specified literal \a value.
     */
    QgsExpressionNodeLiteral( const QVariant &value )
      : mValue( value )
    {}

    //! The value of the literal.
    inline QVariant value() const { return mValue; }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;

    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:
    QVariant mValue;
};

/**
 * An expression node which takes it value from a feature's field.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeColumnRef : public QgsExpressionNode
{
  public:

    /**
     * Constructor for QgsExpressionNodeColumnRef, referencing the column
     * with the specified \a name.
     */
    QgsExpressionNodeColumnRef( const QString &name )
      : mName( name )
      , mIndex( -1 )
    {}

    //! The name of the column.
    QString name() const { return mName; }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes( ) const override; SIP_SKIP

    bool needsGeometry() const override;

    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:
    QString mName;
    int mIndex;
};

/**
 * An expression node for CASE WHEN clauses.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeCondition : public QgsExpressionNode
{
  public:

    /**
     * Represents a "WHEN... THEN..." portation of a CASE WHEN clause in an expression.
     * \ingroup core
     */
    class CORE_EXPORT WhenThen
    {
      public:

        /**
         * A combination of when and then. Simple as that.
         */
        WhenThen( QgsExpressionNode *whenExp, QgsExpressionNode *thenExp );
        ~WhenThen();

        //! WhenThen nodes cannot be copied.
        WhenThen( const WhenThen &rh ) = delete;
        //! WhenThen nodes cannot be copied.
        WhenThen &operator=( const WhenThen &rh ) = delete;

        /**
         * Gets a deep copy of this WhenThen combination.
         */
        QgsExpressionNodeCondition::WhenThen *clone() const SIP_FACTORY;

        /**
         * The expression that makes the WHEN part of the condition.
         * \return The expression node that makes the WHEN part of the condition check.
         */
        QgsExpressionNode *whenExp() const { return mWhenExp; }

        /**
         * The expression node that makes the THEN result part of the condition.
         * \return The expression node that makes the THEN result part of the condition.
         */

        QgsExpressionNode *thenExp() const { return mThenExp; }

      private:
#ifdef SIP_RUN
        WhenThen( const QgsExpressionNodeCondition::WhenThen &rh );
#endif
        QgsExpressionNode *mWhenExp = nullptr;
        QgsExpressionNode *mThenExp = nullptr;

        friend class QgsExpressionNodeCondition;
    };
    typedef QList<QgsExpressionNodeCondition::WhenThen *> WhenThenList;

    /**
     * Create a new node with the given list of \a conditions and an optional \a elseExp expression.
     */
    QgsExpressionNodeCondition( QgsExpressionNodeCondition::WhenThenList *conditions, QgsExpressionNode *elseExp = nullptr );

    /**
     * Create a new node with the given list of \a conditions and an optional \a elseExp expression.
     */
    QgsExpressionNodeCondition( const QgsExpressionNodeCondition::WhenThenList &conditions, QgsExpressionNode *elseExp = nullptr ) SIP_SKIP
  : mConditions( conditions )
    , mElseExp( elseExp )
    {}

    ~QgsExpressionNodeCondition() override;

    QgsExpressionNode::NodeType nodeType() const override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    /**
     * The list of WHEN THEN expression parts of the expression.
     * \return The list of WHEN THEN expression parts of the expression.
     */
    WhenThenList conditions() const { return mConditions; }

    /**
     * The ELSE expression used for the condition.
     * \return The ELSE expression used for the condition.
     */
    QgsExpressionNode *elseExp() const { return mElseExp; }

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;

    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP

    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:
    WhenThenList mConditions;
    QgsExpressionNode *mElseExp = nullptr;
};


#endif // QGSEXPRESSIONNODEIMPL_H
