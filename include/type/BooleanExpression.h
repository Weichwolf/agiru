#pragma once

namespace agiru {

/// \brief Immutable Boolean values evaluated by a braced initializer, left before right.
/// \note This must remain an aggregate: constructor argument evaluation would not
///       provide the same sequencing guarantee. Producer errors propagate before
///       the logical operation; a failing left producer does not run the right one.
struct BooleanOperands {
  const bool left;  ///< Owned left value, captured before evaluating the right expression.
  const bool right; ///< Owned right value, not a reference to mutable AL state.
};

/// \brief Conjunction after both operand producers have completed.
/// \param operands The eagerly evaluated, owned Boolean values.
/// \return True exactly when both values are true.
[[nodiscard]] constexpr bool LogicalAnd(BooleanOperands operands) noexcept {
  return operands.left && operands.right;
}

/// \brief Disjunction after both operand producers have completed.
/// \param operands The eagerly evaluated, owned Boolean values.
/// \return True when either value is true.
[[nodiscard]] constexpr bool LogicalOr(BooleanOperands operands) noexcept {
  return operands.left || operands.right;
}

/// \brief Exclusive disjunction after both operand producers have completed.
/// \param operands The eagerly evaluated, owned Boolean values.
/// \return True exactly when the two values differ.
[[nodiscard]] constexpr bool LogicalXor(BooleanOperands operands) noexcept {
  return operands.left != operands.right;
}

}
