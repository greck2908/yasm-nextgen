//
// YASM location utility functions implementation.
//
//  Copyright (C) 2007  Peter Johnson
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
#include "yasmx/Location_util.h"

#include "util.h"

#include "llvm/ADT/APFloat.h"
#include "yasmx/Support/errwarn.h"
#include "yasmx/Bytecode.h"
#include "yasmx/Expr.h"
#include "yasmx/IntNum.h"
#include "yasmx/Symbol.h"


namespace yasm
{

// Transforms instances of Symbol-Symbol [Symbol+(-1*Symbol)] into single
// ExprTerms if possible.  Uses a simple n^2 algorithm because n is usually
// quite small.  Also works for loc-loc (or Symbol-loc, loc-Symbol).
static void
TransformDistBase(Expr& e, int pos,
                  const FUNCTION::function<bool (ExprTerm& term,
                                                 Location loc1,
                                                 Location loc2)>& func)
{
    ExprTerms& terms = e.getTerms();
    if (pos < 0)
        pos += static_cast<int>(terms.size());

    ExprTerm& root = terms[pos];
    if (!root.isOp(Op::ADD))
        return;

    // Handle symrec-symrec by checking for (-1*symrec)
    // and symrec term pairs (where both symrecs are in the same
    // segment).
    if (root.getNumChild() > 32)
        throw TooComplexError(N_("too many add terms; internal limit of 32"));

    // Yes, this has a maximum upper bound on 32 terms, based on an
    // "insane number of terms" (and ease of implementation) WAG.
    // The right way to do this would be a stack-based alloca, but
    // that's not portable.  We really don't want to alloc
    // here as this function is hit a lot!
    //
    // We use chars to keep things small, as this is a recursive
    // routine and we don't want to eat up stack space.
    unsigned char relpos[32], subpos[32], subneg1[32], subroot[32];
    int num_rel = 0, num_sub = 0;

    // Scan for symrec and (-1*symrec) terms (or location equivalents)
    int n = pos-1;
    while (n >= 0)
    {
        ExprTerm& child = terms[n];
        if (child.isEmpty())
        {
            --n;
            continue;
        }
        if (child.m_depth <= root.m_depth)
            break;
        if (child.m_depth != root.m_depth+1)
        {
            --n;
            continue;
        }

        // Remember symrec terms
        if (child.isType(ExprTerm::SYM | ExprTerm::LOC))
        {
            if ((pos-n) >= 0xff)
                throw TooComplexError(N_("expression too large"));
            relpos[num_rel++] = pos-n;
            --n;
            continue;
        }

        int curpos = n;
        int sym, neg1;
        // Remember (-1*symrec) terms
        if (isNeg1Sym(e, &sym, &neg1, &n, true))
        {
            if ((pos-sym) >= 0xff || (pos-neg1) >= 0xff)
                throw TooComplexError(N_("expression too large"));
            subpos[num_sub] = pos-sym;
            subneg1[num_sub] = pos-neg1;
            subroot[num_sub] = pos-curpos;
            num_sub++;
            continue;
        }

        --n;
    }

    // Match additive and subtractive symbols.
    for (int i=0; i<num_rel; ++i)
    {
        ExprTerm& relterm = terms[pos-relpos[i]];
        SymbolRef rel = relterm.getSymbol();

        for (int j=0; j<num_sub; ++j)
        {
            if (subpos[j] == 0xff)
                continue;   // previously matched
            ExprTerm& subterm = terms[pos-subpos[j]];
            SymbolRef sub = subterm.getSymbol();

            // If it's the same symbol, even if it's external,
            // they should cancel out.
            if (rel && rel == sub)
            {
                relterm.Zero();
                terms[pos-subpos[j]].Clear();
                terms[pos-subneg1[j]].Clear();
                terms[pos-subroot[j]].Zero();
                subpos[j] = 0xff;   // mark as matched
                break;
            }

            Location rel_loc;
            if (rel)
            {
                if (!rel->getLabel(&rel_loc))
                    continue;   // external
            }
            else if (const Location* loc = relterm.getLocation())
                rel_loc = *loc;
            else
                assert(false);

            Location sub_loc;
            if (sub)
            {
                if (!sub->getLabel(&sub_loc))
                    continue;   // external
            }
            else if (const Location* loc = subterm.getLocation())
                sub_loc = *loc;
            else
                assert(false);

            // If both are in the same segment, we leave them in the
            // expression but consider them to "match".
            if (rel_loc.bc->getContainer() !=
                sub_loc.bc->getContainer())
                continue;

            if (func(relterm, sub_loc, rel_loc))
            {
                // Set the matching (-1*Symbol) term to 0
                // (will remove from expression during simplify)
                terms[pos-subpos[j]].Clear();
                terms[pos-subneg1[j]].Clear();
                terms[pos-subroot[j]].Zero();
                subpos[j] = 0xff;   // mark as matched
                break;  // stop looking
            }
        }
    }
}

struct CalcDistFunctor
{
    bool operator() (ExprTerm& term, Location loc, Location loc2)
    {
        IntNum dist;
        if (!CalcDist(loc, loc2, &dist))
            return false;
        // Change the term to an integer
        term = ExprTerm(dist, term.m_depth);
        return true;
    }
};

void
SimplifyCalcDist(Expr& e)
{
    CalcDistFunctor functor;
    e.Simplify(BIND::bind(&TransformDistBase, _1, _2, REF::ref(functor)));
}

struct CalcDistNoBCFunctor
{
    bool operator() (ExprTerm& term, Location loc, Location loc2)
    {
        IntNum dist;
        if (!CalcDistNoBC(loc, loc2, &dist))
            return false;
        // Change the term to an integer
        term = ExprTerm(dist, term.m_depth);
        return true;
    }
};

void
SimplifyCalcDistNoBC(Expr& e)
{
    CalcDistNoBCFunctor functor;
    e.Simplify(BIND::bind(&TransformDistBase, _1, _2, REF::ref(functor)));
}

struct SubstDistFunctor
{
    const FUNCTION::function<void (unsigned int subst,
                                   Location loc,
                                   Location loc2)>& m_func;
    unsigned int m_subst;

    SubstDistFunctor(const FUNCTION::function<void (unsigned int subst,
                                                    Location loc,
                                                    Location loc2)>& func)
        : m_func(func), m_subst(0)
    {}

    bool operator() (ExprTerm& term, Location loc, Location loc2)
    {
        // Call higher-level callback
        m_func(m_subst, loc, loc2);
        // Change the term to an subst
        term = ExprTerm(ExprTerm::Subst(m_subst), term.m_depth);
        m_subst++;
        return true;
    }
};

int
SubstDist(Expr& e,
          const FUNCTION::function<void (unsigned int subst,
                                         Location loc,
                                         Location loc2)>& func)
{
    SubstDistFunctor functor(func);
    e.Simplify(BIND::bind(&TransformDistBase, _1, _2, REF::ref(functor)));
    return functor.m_subst;
}

bool
Evaluate(const Expr& e,
         ExprTerm* result,
         const ExprTerm* subst,
         unsigned int nsubst,
         bool valueloc,
         bool zeroreg)
{
    if (e.isEmpty())
        return false;

    const ExprTerms& terms = e.getTerms();

    // Shortcut the most common case: a single integer or float
    if (terms.size() == 1
        && terms.front().isType(ExprTerm::INT|ExprTerm::FLOAT))
    {
        *result = terms.front();
        return true;
    }

    llvm::SmallVector<ExprTerm, 8> stack;

    for (ExprTerms::const_iterator i=terms.begin(), end=terms.end();
         i != end; ++i)
    {
        const ExprTerm& term = *i;
        if (term.isOp())
        {
            size_t nchild = term.getNumChild();
            assert(stack.size() >= nchild && "not enough terms to evaluate op");
            Op::Op op = term.getOp();
            if (op >= Op::NONNUM)
                return false;

            // Get first child (will be used as result)
            size_t resultindex = stack.size()-nchild;
            ExprTerm& result = stack[resultindex];

            // Calculate through other children
            for (size_t j = resultindex+1; j<stack.size(); ++j)
            {
                ExprTerm& child = stack[j];

                // Promote to float as needed
                if (result.isType(ExprTerm::FLOAT))
                    child.PromoteToFloat(result.getFloat()->getSemantics());
                else if (child.isType(ExprTerm::FLOAT))
                    result.PromoteToFloat(child.getFloat()->getSemantics());

                // Perform calculation
                if (result.isType(ExprTerm::INT))
                    result.getIntNum()->Calc(op, *child.getIntNum());
                else if (op < Op::NEG)
                    CalcFloat(result.getFloat(), op, *child.getFloat());
                else
                    return false;
            }

            // Handle unary op
            if (nchild == 1)
            {
                assert(isUnary(op) && "single-term subexpression is non-unary");
                if (result.isType(ExprTerm::INT))
                    result.getIntNum()->Calc(op);
                else if (op == Op::NEG)
                    result.getFloat()->changeSign();
                else
                    return false;
            }

            // Pop other children off stack, leaving first child as result
            result.m_depth = term.m_depth;
            stack.erase(stack.begin()+resultindex+1, stack.end());
        }
        else if (!term.isEmpty())
        {
            // Convert term to int or float before pushing onto stack.
            // If cannot convert, return false.
            switch (term.getType())
            {
                case ExprTerm::REG:
                    if (!zeroreg)
                        return false;
                    stack.push_back(ExprTerm(0, term.m_depth));
                    break;
                case ExprTerm::SUBST:
                {
                    unsigned int substindex = *term.getSubst();
                    if (!subst || substindex >= nsubst)
                        return false;
                    assert((subst[substindex].getType() == ExprTerm::INT ||
                            subst[substindex].getType() == ExprTerm::FLOAT) &&
                           "substitution must be integer or float");
                    stack.push_back(subst[substindex]);
                    break;
                }
                case ExprTerm::INT:
                case ExprTerm::FLOAT:
                    stack.push_back(term);
                    break;
                case ExprTerm::SYM:
                {
                    Location loc;
                    if (!valueloc || !term.getSymbol()->getLabel(&loc) ||
                        !loc.bc)
                        return false;
                    stack.push_back(ExprTerm(loc.getOffset(), term.m_depth));
                    break;
                }
                case ExprTerm::LOC:
                {
                    Location loc = *term.getLocation();
                    if (!valueloc || !loc.bc)
                        return false;
                    stack.push_back(ExprTerm(loc.getOffset(), term.m_depth));
                    break;
                }
                default:
                    return false;
            }
        }
    }

    assert(stack.size() == 1 && "did not fully evaluate expression");
    result->swap(stack.back());
    return true;
}

} // namespace yasm
