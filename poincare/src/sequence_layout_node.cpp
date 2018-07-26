#include <poincare/sequence_layout_node.h>
#include <poincare/char_layout_node.h>
#include <poincare/horizontal_layout_node.h>
#include <poincare/left_parenthesis_layout_node.h>
#include <poincare/right_parenthesis_layout_node.h>
#include <assert.h>

namespace Poincare {

static inline KDCoordinate max(KDCoordinate x, KDCoordinate y) { return x > y ? x : y; }

void SequenceLayoutNode::moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  if (cursor->position() == LayoutCursor::Position::Left
      && ((lowerBoundLayout() && cursor->layoutReference() == lowerBoundLayout())
        || (upperBoundLayout() && cursor->layoutReference() == upperBoundLayout())))
  {
    // Case: Left of the bounds. Go Left of the sequence.
    cursor->setLayoutNode(this);
    return;
  }
  if (cursor->position() == LayoutCursor::Position::Left
      && argumentLayout()
      && cursor->layoutReference() == argumentLayout())
  {
    // Case: Left of the argument. Go Right of the lower bound.
    assert(lowerBoundLayout() != nullptr);
    cursor->setLayoutNode(lowerBoundLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  assert(cursor->layoutReference() == this);
  if (cursor->position() == LayoutCursor::Position::Right) {
    // Case: Right. Go to the argument and move Left.
    assert(argumentLayout() != nullptr);
    cursor->setLayoutNode(argumentLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  assert(cursor->position() == LayoutCursor::Position::Left);
  // Case: Left. Ask the parent.
  LayoutNode * parentLayout = parent();
  if (parentLayout) {
    parentLayout->moveCursorLeft(cursor, shouldRecomputeLayout);
  }
}

void SequenceLayoutNode::moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  if (cursor->position() == LayoutCursor::Position::Right
      && ((lowerBoundLayout() && cursor->layoutReference() == lowerBoundLayout())
        || (upperBoundLayout() && cursor->layoutReference() == upperBoundLayout())))
  {
    // Case: Right of the bounds. Go Left of the argument.
    assert(argumentLayout() != nullptr);
    cursor->setLayoutNode(argumentLayout());
    cursor->setPosition(LayoutCursor::Position::Left);
    return;
  }
  if (cursor->position() == LayoutCursor::Position::Right
      && argumentLayout()
      && cursor->layoutReference() == argumentLayout())
  {
    // Case: Right of the argument. Go Right.
    cursor->setLayoutNode(this);
    return;
  }
  assert(cursor->layoutReference() == this);
  if (cursor->position() == LayoutCursor::Position::Left) {
    // Case: Left. Go to the upper bound
    assert(upperBoundLayout() != nullptr);
    cursor->setLayoutNode(upperBoundLayout());
    return;
  }
  assert(cursor->position() == LayoutCursor::Position::Right);
  // Case: Right. Ask the parent
  LayoutNode * parentLayout = parent();
  if (parentLayout) {
    return parentLayout->moveCursorRight(cursor, shouldRecomputeLayout);
  }
}

void SequenceLayoutNode::moveCursorUp(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited) {
  if (lowerBoundLayout() && cursor->layoutNode()->hasAncestor(lowerBoundLayout(), true)) {
  // If the cursor is inside the lower bound, move it to the upper bound
    assert(upperBoundLayout() != nullptr);
    upperBoundLayout()->moveCursorUpInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  if (argumentLayout()
      && cursor->isEquivalentTo(LayoutCursor(argumentLayout(), LayoutCursor::Position::Left)))
  {
    // If the cursor is Left of the argument, move it to the upper bound
    assert(upperBoundLayout() != nullptr);
    upperBoundLayout()->moveCursorUpInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  LayoutNode::moveCursorUp(cursor, shouldRecomputeLayout, equivalentPositionVisited);
}

void SequenceLayoutNode::moveCursorDown(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited) {
  if (upperBoundLayout() && cursor->layoutNode()->hasAncestor(upperBoundLayout(), true)) {
    // If the cursor is inside the upper bound, move it to the lower bound
    assert(lowerBoundLayout() != nullptr);
    lowerBoundLayout()->moveCursorDownInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  // If the cursor is Left of the argument, move it to the lower bound
  if (argumentLayout()
      && cursor->isEquivalentTo(LayoutCursor(argumentLayout(), LayoutCursor::Position::Left)))
  {
    assert(lowerBoundLayout() != nullptr);
    lowerBoundLayout()->moveCursorDownInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  LayoutNode::moveCursorDown(cursor, shouldRecomputeLayout, equivalentPositionVisited);
}

void SequenceLayoutNode::deleteBeforeCursor(LayoutCursor * cursor) {
  if (cursor->isEquivalentTo(LayoutCursor(argumentLayout(), LayoutCursor::Position::Left))) {
    // Case: Left of the argument. Delete the layout, keep the argument.
    LayoutRef(this).replaceWith(LayoutRef(argumentLayout()), cursor);
    // WARNING: do not use "this" afterwards
    return;
  }
  LayoutNode::deleteBeforeCursor(cursor);
}


// Protected

void SequenceLayoutNode::computeSize() {
  KDSize nEqualslowerBoundSize = lowerBoundSizeWithNEquals();
  KDSize upperBoundSize = upperBoundLayout()->layoutSize();
  KDSize argumentSize = argumentLayout()->layoutSize();
  KDSize argumentSizeWithParentheses = KDSize(
    argumentSize.width() + 2*ParenthesisLayoutNode::ParenthesisWidth(),
    ParenthesisLayoutNode::HeightGivenChildHeight(argumentSize.height()));
  KDSize result = KDSize(
    max(max(k_symbolWidth, nEqualslowerBoundSize.width()), upperBoundSize.width())+k_argumentWidthMargin+argumentSizeWithParentheses.width(),
    baseline() + max(k_symbolHeight/2+k_boundHeightMargin+nEqualslowerBoundSize.height(), argumentSizeWithParentheses.height() - argumentLayout()->baseline()));
  m_frame.setSize(result);
  m_sized = true;
}

void SequenceLayoutNode::computeBaseline() {
  m_baseline = max(upperBoundLayout()->layoutSize().height()+k_boundHeightMargin+(k_symbolHeight+1)/2, argumentLayout()->baseline());
  m_baselined = true;
}

KDPoint SequenceLayoutNode::positionOfChild(LayoutNode * l) {
  KDSize nEqualslowerBoundSize = lowerBoundSizeWithNEquals();
  KDSize nEqualsSize = KDText::stringSize("n=", k_fontSize);
  KDSize upperBoundSize = upperBoundLayout()->layoutSize();
  KDCoordinate x = 0;
  KDCoordinate y = 0;
  if (l == lowerBoundLayout()) {
    x = nEqualsSize.width() +
      max(max(0, (k_symbolWidth-nEqualslowerBoundSize.width())/2),
          (upperBoundSize.width()-nEqualslowerBoundSize.width())/2);
    y = baseline() + k_symbolHeight/2 + k_boundHeightMargin;
  } else if (l == upperBoundLayout()) {
    x = max(max(0, (k_symbolWidth-upperBoundSize.width())/2), (nEqualslowerBoundSize.width()-upperBoundSize.width())/2);
    y = baseline() - (k_symbolHeight+1)/2- k_boundHeightMargin-upperBoundSize.height();
  } else if (l == argumentLayout()) {
    x = max(max(k_symbolWidth, nEqualslowerBoundSize.width()), upperBoundSize.width())+k_argumentWidthMargin+ParenthesisLayoutNode::ParenthesisWidth();
    y = baseline() - argumentLayout()->baseline();
  } else {
    assert(false);
  }
  return KDPoint(x,y);
}

int SequenceLayoutNode::writeDerivedClassInBuffer(const char * operatorName, char * buffer, int bufferSize, PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const {
  assert(operatorName != nullptr);
  if (bufferSize == 0) {
    return -1;
  }
  buffer[bufferSize-1] = 0;

  // Write the operator name
  int numberOfChar = strlcpy(buffer, operatorName, bufferSize);
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the opening parenthesis
  buffer[numberOfChar++] = '(';
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the argument
  numberOfChar += const_cast<SequenceLayoutNode *>(this)->argumentLayout()->writeTextInBuffer(buffer+numberOfChar, bufferSize-numberOfChar, floatDisplayMode, numberOfSignificantDigits);
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the comma
  buffer[numberOfChar++] = ',';
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the lower bound
  numberOfChar += const_cast<SequenceLayoutNode *>(this)->lowerBoundLayout()->writeTextInBuffer(buffer+numberOfChar, bufferSize-numberOfChar, floatDisplayMode, numberOfSignificantDigits);
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the comma
  buffer[numberOfChar++] = ',';
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the upper bound
  numberOfChar += const_cast<SequenceLayoutNode *>(this)->upperBoundLayout()->writeTextInBuffer(buffer+numberOfChar, bufferSize-numberOfChar, floatDisplayMode, numberOfSignificantDigits);
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the closing parenthesis
  buffer[numberOfChar++] = ')';
  buffer[numberOfChar] = 0;
  return numberOfChar;
}

void SequenceLayoutNode::render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  // Render the "n="
  KDPoint nEqualsPosition = positionOfChild(lowerBoundLayout()).translatedBy(KDPoint(-KDText::stringSize("n=", k_fontSize).width(), 0));
  ctx->drawString("n=", p.translatedBy(nEqualsPosition), k_fontSize, expressionColor, backgroundColor);

  // Render the parentheses
  KDCoordinate argumentWithParenthesesHeight = ParenthesisLayoutNode::HeightGivenChildHeight(argumentLayout()->layoutSize().height());
  KDPoint argumentPosition = positionOfChild(argumentLayout());

  KDPoint leftParenthesisPoint = p.translatedBy(KDPoint(
      argumentPosition.x() - ParenthesisLayoutNode::ParenthesisWidth(),
      argumentPosition.y() + argumentLayout()->layoutSize().height() - argumentWithParenthesesHeight));
  LeftParenthesisLayoutNode::RenderWithChildHeight(argumentWithParenthesesHeight, ctx, leftParenthesisPoint, expressionColor, backgroundColor);

  KDPoint rightParenthesisPoint = p.translatedBy(KDPoint(
      argumentPosition.x() + argumentLayout()->layoutSize().width(),
      argumentPosition.y() + argumentLayout()->layoutSize().height() - argumentWithParenthesesHeight));
  RightParenthesisLayoutNode::RenderWithChildHeight(argumentWithParenthesesHeight, ctx, rightParenthesisPoint, expressionColor, backgroundColor);
}

KDSize SequenceLayoutNode::lowerBoundSizeWithNEquals() {
  KDSize lowerBoundSize = lowerBoundLayout()->layoutSize();
  KDSize nEqualsSize = KDText::stringSize("n=", k_fontSize);
  return KDSize(
      nEqualsSize.width() + lowerBoundSize.width(),
      max(nEqualsSize.height(), lowerBoundSize.height()));
}

}