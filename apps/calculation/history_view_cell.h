#ifndef CALCULATION_HISTORY_VIEW_CELL_H
#define CALCULATION_HISTORY_VIEW_CELL_H

#include <escher.h>
#include "calculation.h"
#include "scrollable_expression_view.h"
#include "../shared/scrollable_exact_approximate_expressions_view.h"

namespace Calculation {

class HistoryViewCell;

class HistoryViewCellDataSource {
public:
  enum class SubviewType {
    Input,
    Output
  };
  HistoryViewCellDataSource();
  void setSelectedSubviewType(HistoryViewCellDataSource::SubviewType subviewType, HistoryViewCell * cell = nullptr);
  SubviewType selectedSubviewType() { return m_selectedSubviewType; }
private:
  SubviewType m_selectedSubviewType;
};

class HistoryViewCell : public ::EvenOddCell, public Responder {
public:
  HistoryViewCell(Responder * parentResponder = nullptr);
  void reloadCell() override;
  void reloadScroll();
  void setEven(bool even) override;
  void setHighlighted(bool highlight) override;
  void setDataSource(HistoryViewCellDataSource * dataSource) { m_dataSource = dataSource; }
  Responder * responder() override {
    return this;
  }
  Poincare::Layout layout() const override;
  KDColor backgroundColor() const override;
  void setCalculation(Calculation * calculation);
  int numberOfSubviews() const override;
  View * subviewAtIndex(int index) override;
  void layoutSubviews() override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;
  constexpr static KDCoordinate k_digitVerticalMargin = 5;
  Shared::ScrollableExactApproximateExpressionsView * outputView();
private:
  constexpr static KDCoordinate k_resultWidth = 80;
  Calculation m_calculation;
  Poincare::Layout m_inputLayout;
  Poincare::Layout m_leftOutputLayout;
  Poincare::Layout m_rightOutputLayout;
  ScrollableExpressionView m_inputView;
  Shared::ScrollableExactApproximateExpressionsView m_scrollableOutputView;
  HistoryViewCellDataSource * m_dataSource;
};

}

#endif
