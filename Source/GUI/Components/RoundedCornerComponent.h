#pragma once

#include <JuceHeader.h>

/** A component that can have rounded corners and will draw within those rounded corners.
 *  It will also check if any parent components have rounded corners and will draw within those
 *  corners as well.
 */
class RoundedCornerComponent : public juce::Component, public juce::ComponentListener
{
public:

    enum ColourIds
    {
        strokeColourId      = 0x200200
    };

    RoundedCornerComponent()
    {
        setColour (strokeColourId, juce::Colours::black);

        clipPath = getRoundedCornersClipPath();
        addComponentListener (this);
    }

    /** Sets a clipping path for this component and all child components.
     *
     * @param radius    The corner radius
     */
    void setRoundedCorners(const float radius)
    {
        getProperties().set("corner-radius", radius);
        repaint();
    }

    void setStroke(float thickness)  { strokeThickness = thickness; }

    //============================================================
    juce::Path* getClipPath() { return clipPath.get(); }

    /** If this component has reduced its bounds by setting the reduce in setRoundedCorners(),
     *  then this will return that reduced rectangle.
     *  @see setRoundedCorners
     */
    juce::Rectangle<int> getReducedLocalBounds() const noexcept
    {
        if (auto reduce = getProperties().getVarPointer ("shadow-reduce"))
            return getLocalBounds().reduced ((int)*reduce);

        return getLocalBounds();
    }

    //============================================================
    /** Will paint within the rounded corners. Everything outside of the rounded
     *  corners will be clipped.
     *
     *  @param g         The graphics context
     */
    virtual void paintWithinCorners([[maybe_unused]] juce::Graphics& g) {}

    void paintOverChildren(juce::Graphics& g) override
    {
        if (clipPath != nullptr && strokeThickness > 0.0f)
        {
            g.setColour (findColour (strokeColourId));
            g.drawRoundedRectangle (roundedCompBounds.toFloat().reduced(strokeThickness * 0.5f), cornerRadius, strokeThickness);
        }
    }

private:

    void paint(juce::Graphics& g) final
    {
        if (clipPath && getLocalBounds().toFloat().intersects (clipPath->getBounds()))
            g.reduceClipRegion (*clipPath);

        paintWithinCorners (g);
    }

    void componentMovedOrResized(Component& component, bool wasMoved, bool wasResized) override
    {
        juce::ignoreUnused (wasMoved, wasResized);

        // Update bounds of the clip path whenever component is moved or resized
        if(&component == this)
            clipPath = getRoundedCornersClipPath();
    }

    /** Finds the rounded corners clip path if set. If not found on the given component, it will look
     *  for clip paths in the parent components.
     *
     *  @return         The clipping path.
     */
    std::unique_ptr<juce::Path> getRoundedCornersClipPath()
    {
        juce::Component* container = this;
        while (container) {
            if (auto radius = container->getProperties().getVarPointer("corner-radius")) {
                auto path = std::make_unique<juce::Path>();
                roundedCompBounds = getLocalArea(container, container->getLocalBounds());
                cornerRadius = (float)*radius;
                path->addRoundedRectangle(roundedCompBounds, cornerRadius);
                return path;
            }
            container = container->getParentComponent();
        }
        roundedCompBounds = {};
        cornerRadius = 0.0f;
        return {};
    }

    std::unique_ptr<juce::Path> clipPath { nullptr };
    juce::Rectangle<int> roundedCompBounds;
    float cornerRadius { 0.0f };
    float strokeThickness { 0.0f };
};