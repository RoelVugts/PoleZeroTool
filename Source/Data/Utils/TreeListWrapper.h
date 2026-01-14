#pragma once

#include <JuceHeader.h>

//==============================================================================
/** A wrapper around a tree that holds a list of other trees.
 *
 * @tparam ObjectType           Should be a sub-class of TreeWrapper
 *
 * @see TreeWrapper
 */
template <typename ObjectType>
class TreeListWrapper : public juce::ValueTree::Listener
{
public:
    using ChildAddCallbackFn = std::function<void (juce::ValueTree&)>;
    using ChildRemovedCallbackFn = std::function<void (juce::ValueTree&, int)>;
    using ChildOrderChangedCallbackFn = std::function<void (int, int)>;

    /** Constructs the TreeListWrapper, creating an array of TreeWrappers of ObjectType.
     *
     * @param parentTree        The tree which this treeWrapper should attach to.
     * @param objectType        The TreeWrapper type ID of the children this tree holds.
     */
    TreeListWrapper (const juce::ValueTree& parentTree, const juce::Identifier& objectType, juce::UndoManager* um = nullptr)
        : parent (parentTree), type (objectType), undoManager (um)
    {
        parent.addListener (this);
        initialise();
    }

    TreeListWrapper (const TreeListWrapper& other) : TreeListWrapper (other.parent, other.type, other.undoManager) {}
    ~TreeListWrapper() override { parent.removeListener (this); }

    //==============================================================================
    void initialise()
    {
        jassert (elements.size() == 0); // must only call this method once at construction

        for (const auto& v : parent)
            if (isSuitableType (v))
                elements.add (new ObjectType (v));
    }

    void setState (const TreeListWrapper& other)
    {
        const int numElements = elements.size();
        const int otherNumElements = other.elements.size();
        const int diff = otherNumElements - numElements;

        for (int i = 0; i < otherNumElements; i++)
        {
            if (i < elements.size())
            {
                elements[i]->setState (*other.elements[i]);
            }
            else
            {
                juce::ValueTree newChild (type);
                ObjectType state (newChild);
                state.setState (other[i]);
                parent.addChild (newChild, i, undoManager);
            }
        }

        // Remove old elements
        for (int i = numElements - 1; i >= (numElements + diff); --i)
            parent.removeChild (i, undoManager);
    }

    //==============================================================================
    /** Returns true if the type of the passed in value tree is the same type as
     *  the objects this TreeListWrapper is holding.
     *
     *  @param v        The value tree to check
     */
    virtual bool isSuitableType (const juce::ValueTree& v) const { return v.hasType (type); }

    //==============================================================================
    ObjectType operator[] (int index) noexcept
    {
        jassert (index < parent.getNumChildren());

        juce::ValueTree child = parent.getChild (index);

        // Invalid index !
        jassert (child.isValid());

        return child;
    }

    const ObjectType operator[] (int index) const noexcept
    {
        jassert (index < parent.getNumChildren());

        juce::ValueTree child = parent.getChild (index);

        // Invalid index !
        jassert (child.isValid());

        return child;
    }

    /** @brief Returns a reference to the TreeWrapper Objects this TreeListWrapper is holding.
     *  If you add callbacks to the TreeWrapper make sure you check the index of the TreeWrapper within the list
     *  since the list might get reordered.\n
     *
     *  Use the [] operator if you want to get the TreeWrapper returned by value.
     *
     *  @param index        Index of the element (child).
     */
    ObjectType& getReference (int index)
    {
        // Index not present in underlying tree !
        jassert (index < parent.getNumChildren());

        // Index has been added to this valueTree wrapper
        jassert (index < elements.size());

        return *elements[index];
    }

    //==============================================================================
    /** Creates and adds a new default child to the valueTree.
     *  This will also trigger a call to the childAdded callbacks.
     *  @param index     index (-1 will add to end of list)
     *  @param um        Undomanager
     */
    void add (int index = -1)
    {
        // Construct new child
        add (new ObjectType({ juce::ValueTree(type) }), index, undoManager);
    }

    /** Adds the passed in child to the valueTree.
     *  This will also trigger a call to the childAdded callbacks.
     *  @param newChild  The new child to add, the array will take ownership of the object so make sure to not delete it somewhere else
     *  @param index     The index to insert (-1 will add at the end of the list)
     *  @param um        UndoManager
     */
    void add (ObjectType* newChild, int index = -1)
    {
        // Adding at an index greater than the end of the array ?
        jassert (index < (elements.size() + 1));

        if (index == parent.getNumChildren() || index < 0)
            elements.add (newChild);
        else
            elements.insert (index, newChild);

        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        parent.addChild (newChild->getTree(), index, undoManager);
    }

    /** Removes a child from the valueTree at a given index.
     *  @param index             The index to remove.
     *  @param um                UndoManager
     */
    void remove (int index)
    {
        jassert (parent.getChild (index).isValid());
        jassert (index < elements.size());

        elements.remove (index);

        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        parent.removeChild (index, undoManager);
    }

    /** Moves a child from one index to another.
     *  This is not a swap operation!
     *  @param currentIndex         The old index of the child
     *  @param newIndex             The new index of the child
     *  @param um                   UndoManager
     */
    void moveChild (int currentIndex, int newIndex) { parent.moveChild (currentIndex, newIndex, undoManager); }

    //==============================================================================
    /** Returns the number of children the underlying ValueTree holds.*/
    int size() const noexcept { return parent.getNumChildren(); }

    /** Returns the index of the child within the tree.
     *
     *  @param v    The valueTree child to get the index for
     *  @returns    The index wihtin the tree or -1 if it could not be found.
     */
    int indexOf (const juce::ValueTree& v) const noexcept
    {
        for (int i = 0; i < elements.size(); ++i)
            if (elements[i]->getTree() == v)
                return i;

        return -1;
    }

    juce::ValueTree& getParentTree() { return parent; }
    const juce::ValueTree& getParentTree() const { return parent; }

    //==============================================================================
    /** Add a callback whenever a child is added to this tree.
     * @param f             The callback function. (juce::ValueTree& addedChild) -> void.
     */
    template <typename Fn>
    void setOnChildAdded (Fn f)
    {
        childAddedLambdas.emplace_back (std::forward<Fn> (f));
    }

    /** Add a callback whenever a child is removed from this tree.
     * @param f             The callback function. (juce::ValueTree& addedChild, int index) -> void.
     */
    template <typename Fn>
    void setOnChildRemoved (Fn f)
    {
        childRemovedLambdas.emplace_back (std::forward<Fn> (f));
    }

    /** Add a callback whenever the child order is changed.
     * @param f             The callback function. (int oldIndex, int newIndex) -> void.
     */
    template <typename Fn>
    void setOnChildOrderChanged (Fn f)
    {
        childOrderChangedLambdas.emplace_back (std::forward<Fn> (f));
    }

private:
    //==============================================================================
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& child) override
    {
        if (isChildTree (child))
        {
            const int index = parentTree.indexOf (child);
            jassert (index >= 0);

            if (! ignoreCallbacks)
            // Don't add to array if we added a child via the add() function in this instance
            // since it is already added then
            {
                if (index == parentTree.getNumChildren() - 1)
                    elements.add (new ObjectType (child));
                else
                    elements.addSorted (*this, new ObjectType (child));
            }

            for (ChildAddCallbackFn& f : childAddedLambdas)
                juce::NullCheckedInvocation::invoke (f, child);
        }
    }

    void valueTreeChildRemoved (juce::ValueTree& exParent, juce::ValueTree& child, int index) override
    {
        if (parent == exParent && isSuitableType (child))
        {
            if (! ignoreCallbacks)
            // Don't remove from array if we removed a child via the remove() function in this instance
            // since it is already removed then
            {
                const int oldIndex = indexOf (child);

                if (oldIndex >= 0)
                    elements.remove (oldIndex);
            }

            for (ChildRemovedCallbackFn& f : childRemovedLambdas)
                juce::NullCheckedInvocation::invoke (f, child, index);
        }
    }

    void valueTreeChildOrderChanged (juce::ValueTree& tree, int oldIndex, int newIndex) override
    {
        if (tree == parent)
            sortArray();

        for (ChildOrderChangedCallbackFn& f : childOrderChangedLambdas)
            juce::NullCheckedInvocation::invoke (f, oldIndex, newIndex);
    }

    void valueTreePropertyChanged (juce::ValueTree& v, const juce::Identifier& id) override
    {
        juce::ignoreUnused(v, id);
        // Handle this in the TreeWrapper this list holds
    }

    void valueTreeParentChanged (juce::ValueTree&) override {}
    void valueTreeRedirected (juce::ValueTree&) override { jassertfalse; } // may need to add handling if this is hit

protected:
    void deleteAllObjects() { elements.clear(); }
    bool isChildTree (juce::ValueTree& v) const { return isSuitableType (v) && v.getParent() == parent; }
    void sortArray() { elements.sort (*this); }

    juce::ValueTree parent;
    juce::Identifier type;
    juce::OwnedArray<ObjectType> elements;

    std::vector<ChildAddCallbackFn> childAddedLambdas;
    std::vector<ChildRemovedCallbackFn> childRemovedLambdas;
    std::vector<ChildOrderChangedCallbackFn> childOrderChangedLambdas;

    juce::UndoManager* undoManager { nullptr };

    bool ignoreCallbacks { false };

public:
    int compareElements (const ObjectType* first, const ObjectType* second) const
    {
        int index1 = parent.indexOf (first->getTree());
        int index2 = parent.indexOf (second->getTree());
        return index1 - index2;
    }
};