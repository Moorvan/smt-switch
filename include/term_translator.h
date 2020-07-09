/*********************                                                        */
/*! \file term_translator.h
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Class for translating terms from one solver to another. Keeps
**        a cache so it can be called multiple times (without redeclaring
**        symbols, which would throw an exception).
**/

#ifndef SMT_TERM_TRANSLATOR_H
#define SMT_TERM_TRANSLATOR_H

#include <unordered_map>

#include "smt_defs.h"
#include "solver.h"
#include "sort.h"
#include "term.h"

namespace smt {

/** Class for translating terms from *one* other solver to *one* new solver
 *  will fail if you try to convert terms from more than one solver
 *  This class has no reference to the other solver because it's not needed
 *  it only needs the solver that's being transferred *to* so that it can
 *  make sorts and terms.
 *  Because symbols can only be declared once, there will be errors
 *  if the symbol is already declared in the new solver. To avoid this
 *  populate the TermTranslator's cache with a mapping from
 *  <other solver's symbol> -> <new solver's symbol>
 */
class TermTranslator
{
 public:
  TermTranslator(const SmtSolver & s) : solver(s) {}
  /** Transfers a sort from the other solver to this solver
   *  @param the sort transfer
   *  @return a sort belonging to this solver
   */
  Sort transfer_sort(const Sort & sort) const;

  /** Transfers a term from the other solver to this solver
   *  @param term the term to transfer to the member variable solver
   *  @return a term belonging to this solver
   */
  Term transfer_term(const Term & term);

  /** Transfers a term and casts it to a particular SortKind
   *  for now, only supports Bool <-> BV1 and Int <-> Real
   *  will throw an exception if something else is requested
   *  @param term the term to transfer to the member variable solver
   *  @param sk the expected SortKind of the transferred term
   *  @return a term belonging to this solver
   */
  Term transfer_term(const Term & term, const SortKind sk);

  /* Returns reference to cache -- can be used to populate with symbols */
  UnorderedTermMap & get_cache() { return cache; };

  /* Returns a reference to the solver this object translates terms to */
  const SmtSolver & get_solver() { return solver; };

 protected:
  /** Creates a term value from a string of the given sort
   *  @param val the string representation of the value
   *  @param orig_sort the sort from the original solver (transfer_sort is
   *  called in this function)
   *  @return a term with the given value
   */
  Term value_from_smt2(const std::string val, const Sort sort) const;

  /** identifies relevant casts to perform an operation
   *  assumes the operation is currently not well-sorted
   *  e.g. check_sortedness returns false
   *  could be more general in the future, for now focusing on
   *  Bool / BV1 case
   *  It can either change the operator or cast the terms
   *  @param op the operator that should be applied
   *  @param terms the terms to apply it to
   *  @return a well-sorted term with an operator applied to casted terms
   *  Note: the operator can change, e.g. BVAnd -> And
   *  This method uses cast_term
   */
  Term cast_op(Op op, const TermVec & terms) const;

  /** casts a term to a different sort
   *  could be more general in future, for now just does a few common
   * conversions such as: Bool <-> BV1 Int  <-> Real
   *  @param term the term to cast
   *  @param the sort to cast it to
   *  @return the new term
   *  throws a NotImplementedException if it cannot interpret the cast
   *  the term and sort MUST belong to the same solver
   */
  Term cast_term(const Term & term, const Sort & sort) const;

  /** casts a value term to a different sort
   *  could be more general in future, for now just does
   * conversions such as: Bool <-> BV1
   *  @param val the value term to cast
   *  @param the sort to cast it to
   *  @return the new term
   *  throws a NotImplementedException if it cannot interpret the cast
   *  the term and sort MUST belong to the same solver
   */
  Term cast_value(const Term & term, const Sort & sort) const;

  // Note: const meaning the solver doesn't change to a different solver
  // it can still call non-const methods of the solver
  const SmtSolver & solver;  ///< solver to translate terms to
  UnorderedTermMap cache;
};
}  // namespace smt

#endif
