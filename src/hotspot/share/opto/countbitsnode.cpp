/*
 * Copyright (c) 2014, 2025, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "opto/countbitsnode.hpp"
#include "opto/opcodes.hpp"
#include "opto/phaseX.hpp"
#include "opto/type.hpp"
#include "utilities/count_leading_zeros.hpp"
#include "utilities/count_trailing_zeros.hpp"

static int count_leading_zeros_int(jint i) {
  return i == 0 ? BitsPerInt : count_leading_zeros(i);
}

static int count_leading_zeros_long(jlong l) {
  return l == 0 ? BitsPerLong : count_leading_zeros(l);
}

static int count_trailing_zeros_int(jint i) {
  return i == 0 ? BitsPerInt : count_trailing_zeros(i);
}

static int count_trailing_zeros_long(jlong l) {
  return l == 0 ? BitsPerLong : count_trailing_zeros(l);
}

//------------------------------Value------------------------------------------
const Type* CountLeadingZerosINode::Value(PhaseGVN* phase) const {
  const Type* t = phase->type(in(1));
  if (t == Type::TOP) {
    return Type::TOP;
  }

  // To minimize `count_leading_zeros(x)`, we should make the highest 1 bit in x
  // as far to the left as possible. A bit in x can be 1 iff this bit is not
  // forced to be 0, i.e. the corresponding bit in `x._bits._zeros` is 0. Thus:
  //   min(clz(x)) = number of bits to the left of the highest 0 bit in x._bits._zeros
  //               = count_leading_ones(x._bits._zeros) = clz(~x._bits._zeros)
  //
  // To maximize `count_leading_zeros(x)`, we should make the leading zeros as
  // many as possible. A bit in x can be 0 iff this bit is not forced to be 1,
  // i.e. the corresponding bit in `x._bits._ones` is 0. Thus:
  //   max(clz(x)) = clz(x._bits._ones)
  //
  // Therefore, the range of `count_leading_zeros(x)` is:
  //   [clz(~x._bits._zeros), clz(x._bits._ones)]
  //
  // A more detailed proof using Z3 can be found at:
  //   https://github.com/openjdk/jdk/pull/25928#discussion_r2256750507
  const TypeInt* ti = t->is_int();
  return TypeInt::make(count_leading_zeros_int(~ti->_bits._zeros),
                       count_leading_zeros_int(ti->_bits._ones),
                       ti->_widen);
}

//------------------------------Value------------------------------------------
const Type* CountLeadingZerosLNode::Value(PhaseGVN* phase) const {
  const Type* t = phase->type(in(1));
  if (t == Type::TOP) {
    return Type::TOP;
  }

  // The proof of correctness is same as the above comments
  // in `CountLeadingZerosINode::Value`.
  const TypeLong* tl = t->is_long();
  return TypeInt::make(count_leading_zeros_long(~tl->_bits._zeros),
                       count_leading_zeros_long(tl->_bits._ones),
                       tl->_widen);
}

//------------------------------Value------------------------------------------
const Type* CountTrailingZerosINode::Value(PhaseGVN* phase) const {
  const Type* t = phase->type(in(1));
  if (t == Type::TOP) {
    return Type::TOP;
  }

  // To minimize `count_trailing_zeros(x)`, we should make the lowest 1 bit in x
  // as far to the right as possible. A bit in x can be 1 iff this bit is not
  // forced to be 0, i.e. the corresponding bit in `x._bits._zeros` is 0. Thus:
  //   min(ctz(x)) = number of bits to the right of the lowest 0 bit in x._bits._zeros
  //               = count_trailing_ones(x._bits._zeros) = ctz(~x._bits._zeros)
  //
  // To maximize `count_trailing_zeros(x)`, we should make the trailing zeros as
  // many as possible. A bit in x can be 0 iff this bit is not forced to be 1,
  // i.e. the corresponding bit in `x._bits._ones` is 0. Thus:
  //   max(ctz(x)) = ctz(x._bits._ones)
  //
  // Therefore, the range of `count_trailing_zeros(x)` is:
  //   [ctz(~x._bits._zeros), ctz(x._bits._ones)]
  //
  // A more detailed proof using Z3 can be found at:
  //   https://github.com/openjdk/jdk/pull/25928#discussion_r2256750507
  const TypeInt* ti = t->is_int();
  return TypeInt::make(count_trailing_zeros_int(~ti->_bits._zeros),
                       count_trailing_zeros_int(ti->_bits._ones),
                       ti->_widen);
}

//------------------------------Value------------------------------------------
const Type* CountTrailingZerosLNode::Value(PhaseGVN* phase) const {
  const Type* t = phase->type(in(1));
  if (t == Type::TOP) {
    return Type::TOP;
  }

  // The proof of correctness is same as the above comments
  // in `CountTrailingZerosINode::Value`.
  const TypeLong* tl = t->is_long();
  return TypeInt::make(count_trailing_zeros_long(~tl->_bits._zeros),
                       count_trailing_zeros_long(tl->_bits._ones),
                       tl->_widen);
}
