#pragma once

#include "../../Data/PoleZeroState.h"

#include <JuceHeader.h>

class PoZeTableModel : public juce::TableListBoxModel
{
public:

    enum
    {
        typeColumnId    = 1,
        magColumnId     = 2,
        angleColumnId   = 3

    };

    PoZeTableModel(PoleZeroState settings, juce::TableListBox& owner_) : state(settings), owner(owner_)
    {
    }

    int getNumRows() override
    {
        return state.points.size();
    }

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        juce::ignoreUnused(rowNumber, width, height, rowIsSelected);

        g.fillAll(juce::Colours::black);
    }

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        // This table does not do any painting of cells. Instead it uses custom components by implementing
        // refreshComponentForCell().
        juce::ignoreUnused(g, rowNumber, columnId, width, height, rowIsSelected);
    }

    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override
    {
        juce::ignoreUnused(isRowSelected);

        const juce::Font::FontStyleFlags style = columnId == typeColumnId ? juce::Font::bold : juce::Font::plain;
        const juce::Font font(owner.withDefaultMetrics (FontOptions ((float) owner.getRowHeight() * 0.7f, style)));

        auto* editor = dynamic_cast<juce::TextEditor*>(existingComponentToUpdate);
        if (editor == nullptr)
        {
            editor = new juce::TextEditor("Cell Editor");
            editor->setColour (juce::TextEditor::ColourIds::textColourId, owner.findColour (juce::ListBox::ColourIds::textColourId));
            editor->setColour (juce::TextEditor::ColourIds::backgroundColourId, LAF::Colours::buttonColour);
            editor->setColour (juce::TextEditor::ColourIds::outlineColourId, LAF::Colours::buttonOutlineColour);
            editor->setColour (juce::TextEditor::ColourIds::focusedOutlineColourId, LAF::Colours::highlightedColour);
            editor->setFont (font);
            editor->setJustification (juce::Justification::centredLeft);
            editor->setTooltip (owner.getTooltip());

            editor->onReturnKey = [=, this]() {
                const juce::String& text = editor->getText();
                PointState pointState = state.points[rowNumber];

                switch (columnId)
                {
                    case typeColumnId:
                    {
                        if (text.equalsIgnoreCase ("pole"))
                            pointState.pointType.setValue (PoZePlot::Point::Type::pole);
                        else if (text.equalsIgnoreCase ("zero"))
                            pointState.pointType.setValue (PoZePlot::Point::Type::zero);
                        else
                            editor->setText ((pointState.pointType.getValue() == PoZePlot::Point::Type::pole) ? "Pole" : "Zero");
                        break;
                    }
                    case magColumnId:
                    {
                        std::complex<double> complex = { pointState.real.getValue(), pointState.imag.getValue() };
                        const double mag = text.getDoubleValue();
                        const double angle = std::arg(complex);
                        complex = std::polar(mag, angle);
                        pointState.real.setValue (std::clamp(complex.real(), minPoZePlotRange, maxPoZePlotRange));
                        pointState.imag.setValue (std::clamp(complex.imag(), minPoZePlotRange, maxPoZePlotRange));
                        break;
                    }
                    case angleColumnId:
                    {
                        std::complex<double> complex = { pointState.real.getValue(), pointState.imag.getValue() };
                        const double mag = std::abs(complex);
                        const double angle = text.getDoubleValue() * juce::MathConstants<double>::pi;
                        complex = std::polar(mag, angle);
                        pointState.real.setValue (std::clamp(complex.real(), minPoZePlotRange, maxPoZePlotRange));
                        pointState.imag.setValue (std::clamp(complex.imag(), minPoZePlotRange, maxPoZePlotRange));
                        break;
                    }

                    default: jassertfalse; break;
                }

                editor->giveAwayKeyboardFocus();
            };
        }

        PointState pointState = state.points[rowNumber];
        juce::String text;

        int digitWidth = juce::GlyphArrangement::getStringWidthInt (font, "8");
        int rowWidth = editor->getBounds().getWidth();
        const int numDecimals = std::max((int)(std::ceil((double) rowWidth / (double)digitWidth)) - 5, 10);

        switch (columnId)
        {
            case typeColumnId:
            {
                text = (pointState.pointType.getValue() == PoZePlot::Point::Type::pole) ? "Pole" : "Zero";
                break;
            }

            case magColumnId:
            {
                const double mag = std::abs( std::complex<double>{ pointState.real.getValue(), pointState.imag.getValue() });
                text = juce::String(mag, numDecimals);
                break;
            }

            case angleColumnId:
            {
                const double angle = std::arg (std::complex<double>{ pointState.real.getValue(), pointState.imag.getValue() }) / juce::MathConstants<double>::pi;
                text = juce::String (angle, numDecimals);
                break;
            }

            default: jassertfalse; break;
        }

        editor->setText (text, juce::dontSendNotification);

        return editor;
    }

private:

    PoleZeroState state;
    juce::TableListBox& owner;
};

class PoZeTable : public juce::TableListBox, private juce::AsyncUpdater
{
public:
    PoZeTable(PoleZeroState settings) : state(settings)
    {
        getHeader().addColumn ("Type", PoZeTableModel::typeColumnId, 50, 30, -1, TableHeaderComponent::ColumnPropertyFlags::notSortable);
        getHeader().addColumn ("Mag", PoZeTableModel::magColumnId, 50, 30, -1, TableHeaderComponent::ColumnPropertyFlags::notSortable);
        getHeader().addColumn ("Angle", PoZeTableModel::angleColumnId, 50, 30, -1, TableHeaderComponent::ColumnPropertyFlags::notSortable);
        getHeader().setStretchToFitActive (true);
        setRowHeight (18);

        getHeader().setColour (juce::TableHeaderComponent::ColourIds::textColourId, LAF::Colours::textColour);
        getHeader().setColour (juce::TableHeaderComponent::ColourIds::outlineColourId, LAF::Colours::buttonOutlineColour);
        getHeader().setColour (juce::TableHeaderComponent::ColourIds::highlightColourId, LAF::Colours::highlightedColour);
        getHeader().setColour (juce::TableHeaderComponent::ColourIds::backgroundColourId, LAF::Colours::secondaryColour);

        setColour (juce::ListBox::ColourIds::textColourId, LAF::Colours::textColour);
        setColour (juce::ListBox::ColourIds::backgroundColourId, LAF::Colours::primaryColour);
        setColour (juce::ListBox::ColourIds::outlineColourId, LAF::Colours::buttonOutlineColour);

        getVerticalScrollBar().setColour (juce::ScrollBar::ColourIds::thumbColourId, juce::Colours::grey);

        model = std::make_unique<PoZeTableModel>(settings, *this);
        setModel (model.get());

        state.setOnAnyPropertyChanged ([this](juce::ValueTree&) {
            triggerAsyncUpdate();
        });

        state.points.setOnChildAdded ([this](juce::ValueTree&) {
            triggerAsyncUpdate();
        });

        state.points.setOnChildRemoved ([this](juce::ValueTree&, int) {
            triggerAsyncUpdate();
        });
    }

    ~PoZeTable() override
    {
        setModel (nullptr);
    }

    void paintOverChildren(Graphics& g) override
    {
        auto bounds = getLocalBounds();
        int headerHeight = getHeaderHeight();
        int contentHeight = getRowHeight() * getNumRows();
        auto outlineArea = bounds.removeFromTop (headerHeight + contentHeight);

        g.setColour (LAF::Colours::buttonOutlineColour);
        g.drawRect (outlineArea, 2);
    }

private:

    void handleAsyncUpdate() override
    {
        updateContent();
        repaint();
    }

    PoleZeroState state;
    std::unique_ptr<PoZeTableModel> model;
};