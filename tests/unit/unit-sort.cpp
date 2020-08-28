/*********************                                                        */
/*! \file unit-sort.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Unit tests for sorts
**
**
**/

#include <utility>
#include <vector>

#include "available_solvers.h"
#include "gtest/gtest.h"
#include "smt.h"

using namespace smt;
using namespace std;

namespace smt_tests {

TEST(SortKind, ToString)
{
  EXPECT_EQ(to_string(ARRAY), "ARRAY");
  EXPECT_EQ(to_string(BOOL), "BOOL");
  EXPECT_EQ(to_string(BV), "BV");
  EXPECT_EQ(to_string(INT), "INT");
  EXPECT_EQ(to_string(REAL), "REAL");
  EXPECT_EQ(to_string(FUNCTION), "FUNCTION");
  EXPECT_EQ(to_string(UNINTERPRETED), "UNINTERPRETED");
}

class UnitSortTests : public ::testing::Test,
                      public ::testing::WithParamInterface<SolverEnum>
{
 protected:
  void SetUp() override
  {
    s = create_solver(GetParam());

    boolsort = s->make_sort(BOOL);
    bvsort = s->make_sort(BV, 4);
    funsort = s->make_sort(FUNCTION, SortVec{ bvsort, bvsort });
    arrsort = s->make_sort(ARRAY, bvsort, bvsort);
  }
  SmtSolver s;
  Sort boolsort, bvsort, funsort, arrsort;
};

class UnitSortArithTests : public UnitSortTests
{
 protected:
  void SetUp() override
  {
    UnitSortTests::SetUp();
    intsort = s->make_sort(INT);
    realsort = s->make_sort(REAL);
  }
  Sort intsort, realsort;
};

TEST_P(UnitSortTests, SameSortDiffObj)
{
  Sort boolsort_2 = s->make_sort(BOOL);
  EXPECT_EQ(boolsort->hash(), boolsort_2->hash());
  EXPECT_EQ(boolsort, boolsort_2);

  Sort bvsort_2 = s->make_sort(BV, 4);
  EXPECT_EQ(bvsort->hash(), bvsort_2->hash());
  EXPECT_EQ(bvsort, bvsort_2);

  Sort funsort_2 = s->make_sort(FUNCTION, { bvsort, bvsort_2 });
  EXPECT_EQ(funsort->hash(), funsort_2->hash());
  EXPECT_EQ(funsort, funsort_2);

  Sort arrsort_2 = s->make_sort(ARRAY, bvsort, bvsort_2);
  EXPECT_EQ(arrsort->hash(), arrsort_2->hash());
  EXPECT_EQ(arrsort, arrsort_2);
}

TEST_P(UnitSortTests, SortParams)
{
  EXPECT_EQ(bvsort->get_width(), 4);
  EXPECT_EQ(arrsort->get_indexsort(), bvsort);
  EXPECT_EQ(arrsort->get_elemsort(), bvsort);
  // not every solver supports querying function types for domain/codomain yet
}

TEST_P(UnitSortTests, UninterpretedSort)
{
  Sort uninterp_sort;
  try
  {
    uninterp_sort = s->make_sort("declared-sort", 0);
  }
  catch (SmtException & e)
  {
    // if not supported, that's fine.
    std::cout << "got exception when declaring sort: " << e.what() << std::endl;
    return;
  }

  ASSERT_TRUE(uninterp_sort);
  EXPECT_EQ(uninterp_sort->get_sort_kind(), UNINTERPRETED);
  EXPECT_EQ(uninterp_sort->get_arity(), 0);

  // Now try non-zero arity (not supported by very many solvers)
  Sort sort_cons;
  try
  {
    sort_cons = s->make_sort("sort-con", 4);
  }
  catch (SmtException & e)
  {
    // if not supported, that's fine.
    std::cout << "got exception when declaring nonzero arity sort: " << e.what()
              << std::endl;
    return;
  }

  ASSERT_TRUE(sort_cons);
  // Expecting an uninterpreted constructor sort
  EXPECT_EQ(sort_cons->get_sort_kind(), UNINTERPRETED_CONS);
  EXPECT_EQ(sort_cons->get_arity(), 4);

  // now try applying the sort constructor
  Sort applied_sort_cons =
      s->make_sort(sort_cons, { bvsort, bvsort, bvsort, boolsort });
  EXPECT_EQ(applied_sort_cons->get_arity(), 0);
  std::cout << "before param_sorts with " << s->get_solver_enum() << std::endl;
  SortVec param_sorts = applied_sort_cons->get_uninterpreted_param_sorts();
  EXPECT_EQ(param_sorts.size(), 4);
  EXPECT_EQ(param_sorts[0], bvsort);
  EXPECT_EQ(param_sorts[1], bvsort);
  EXPECT_EQ(param_sorts[2], bvsort);
  EXPECT_EQ(param_sorts[3], boolsort);
}

TEST_P(UnitSortArithTests, SameSortDiffObj)
{
  Sort intsort_2 = s->make_sort(INT);
  EXPECT_EQ(intsort->hash(), intsort_2->hash());
  EXPECT_EQ(intsort, intsort_2);

  Sort realsort_2 = s->make_sort(REAL);
  EXPECT_EQ(realsort->hash(), realsort_2->hash());
  EXPECT_EQ(realsort, realsort_2);
}

INSTANTIATE_TEST_SUITE_P(ParameterizedUnitSortTests,
                         UnitSortTests,
                         testing::ValuesIn(available_solver_enums()));

}  // namespace smt_tests
