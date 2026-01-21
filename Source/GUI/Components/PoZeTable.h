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
            const auto defaultTextColour = owner.findColour (ListBox::textColourId);
            editor->setColour (juce::TextEditor::ColourIds::textColourId, defaultTextColour);
            editor->setColour (juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::black);
            editor->setFont (font);
            editor->setJustification (juce::Justification::centredLeft);

            editor->onReturnKey = [=]() {
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
                        const double mag = text.getFloatValue();
                        const double angle = std::arg(complex);
                        complex = std::polar(mag, angle);
                        pointState.real.setValue (complex.real());
                        pointState.imag.setValue (complex.imag());
                        break;
                    }
                    case angleColumnId:
                    {
                        std::complex<double> complex = { pointState.real.getValue(), pointState.imag.getValue() };
                        const double mag = std::abs(complex);
                        const double angle = text.getFloatValue();
                        complex = std::polar(mag, angle);
                        pointState.real.setValue (complex.real());
                        pointState.imag.setValue (complex.imag());
                        break;
                    }

                    default: jassertfalse; break;
                }
            };
        }

        PointState pointState = state.points[rowNumber];
        juce::String text;

        int digitWidth = juce::GlyphArrangement::getStringWidthInt (font, "8");
        int rowWidth = editor->getBounds().getWidth();
        const int numDecimals = std::max((int)((double) rowWidth / digitWidth) - 5, 2);

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
                const double angle = std::arg (std::complex<double>{ pointState.real.getValue(), pointState.imag.getValue() });
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

class PoZeTable : public juce::TableListBox
{
public:
    PoZeTable(PoleZeroState settings) : state(settings)
    {
        getHeader().addColumn ("Type", PoZeTableModel::typeColumnId, 50, 30);
        getHeader().addColumn ("Mag", PoZeTableModel::magColumnId, 50, 30);
        getHeader().addColumn ("Angle", PoZeTableModel::angleColumnId, 50, 30);
        getHeader().setStretchToFitActive (true);
        setRowHeight (18);

        setColour (juce::ListBox::textColourId, juce::Colours::white);

        model = std::make_unique<PoZeTableModel>(settings, *this);
        setModel (model.get());

        state.setOnAnyPropertyChanged ([this](juce::ValueTree&) {
            updateContent();
            repaint();
        });

        state.points.setOnChildAdded ([this](juce::ValueTree&) {
            updateContent();
            repaint();
        });

        state.points.setOnChildRemoved ([this](juce::ValueTree&, int) {
            updateContent();
            repaint();
        });
    }

    ~PoZeTable() override
    {
        setModel (nullptr);
    }

    PoleZeroState state;
    std::unique_ptr<PoZeTableModel> model;
};