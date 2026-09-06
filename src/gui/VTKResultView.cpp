#include "gui/VTKResultView.hpp"

#include <QPainter>

#include <algorithm>
#include <cmath>

VTKResultView::VTKResultView(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(240);
}

void VTKResultView::setResult(cfd::VTKResult result)
{
    result_ = std::move(result);
    hasResult_ = true;
    update();
}

void VTKResultView::setField(VTKDisplayField field)
{
    field_ = field;
    update();
}

void VTKResultView::clear()
{
    result_ = {};
    hasResult_ = false;
    update();
}

const std::vector<double>& VTKResultView::displayedValues() const
{
    if (field_ == VTKDisplayField::Pressure) return result_.pressure;
    if (field_ == VTKDisplayField::Vorticity) return result_.vorticity;
    return result_.velocityMagnitude;
}

void VTKResultView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette().base());
    if (!hasResult_)
    {
        painter.drawText(rect(), Qt::AlignCenter, "No VTK result loaded");
        return;
    }

    const std::vector<double>& values = displayedValues();
    const auto [minimum, maximum] = std::minmax_element(values.begin(), values.end());
    const double range = *maximum - *minimum;
    const QRectF drawingArea = rect().adjusted(8, 8, -8, -8);
    const double cellWidth = drawingArea.width() / static_cast<double>(result_.nx);
    const double cellHeight = drawingArea.height() / static_cast<double>(result_.ny);

    painter.setPen(Qt::NoPen);
    for (std::size_t row = 0; row < result_.ny; ++row)
    {
        for (std::size_t column = 0; column < result_.nx; ++column)
        {
            const std::size_t index = row * result_.nx + column;
            const double normalized = range > 0.0 ? (values[index] - *minimum) / range : 0.5;
            painter.setBrush(QColor::fromHsvF(0.66 * (1.0 - normalized), 0.85, 0.95));
            painter.drawRect(QRectF(
                drawingArea.left() + static_cast<double>(column) * cellWidth,
                drawingArea.top() + static_cast<double>(result_.ny - row - 1) * cellHeight,
                cellWidth + 0.5,
                cellHeight + 0.5
            ));
        }
    }
}