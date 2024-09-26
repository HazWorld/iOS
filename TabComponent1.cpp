#include "TabComponent1.hpp"

//==============================================================================
// TabComponent1 implementation (Chord Display)
TabComponent1::TabComponent1()
{
    // Set up the chord label UI
    addAndMakeVisible(chordLabel);
    chordLabel.setText("Select a chord...", juce::dontSendNotification);
    chordLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    chordLabel.setJustificationType(juce::Justification::centred);

    // Set up the chord ComboBox for selecting chords
    chordComboBox.addItem("C Major", 1);
    chordComboBox.addItem("G Major", 2);
    chordComboBox.addItem("D Major", 3);
    chordComboBox.addItem("A Major", 4);
    chordComboBox.addItem("E Major", 5);
    chordComboBox.addItem("A Minor", 6);
    chordComboBox.addItem("E Minor", 7);

    chordComboBox.onChange = [this]() { loadChord(); };  // Callback when a new chord is selected
    addAndMakeVisible(chordComboBox);

    // Initialize the layout and size of the component
    setSize(500, 300);  // Reduced size to fit everything on screen
}

void TabComponent1::paint(juce::Graphics& g)
{
    // Set background color for the fretboard area
    g.fillAll(juce::Colour::fromRGB(240, 230, 200));  // Light wood texture background

    // Draw frets with gradient and shadows for a 3D look
    juce::ColourGradient fretGradient(juce::Colours::lightgrey, 0, 0, juce::Colours::darkgrey, getWidth(), 0, false);
    g.setGradientFill(fretGradient);

    // Draw fret lines (4 frets, reduced spacing)
    for (int i = 0; i < 4; ++i)  // Limiting to 4 frets
    {
        int fretX = 80 + i * 80;  // Reduced spacing for frets
        g.setColour(juce::Colours::darkgrey);
        g.drawLine(fretX, 80, fretX, 260, 3.0f);  // Frets are smaller, fit to screen

        // Add fret numbers just above the fret lines
        g.setColour(juce::Colours::black);
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));  // Reduced font size for numbers
        g.drawText(juce::String(i + 1), fretX - 5, 60, 20, 20, juce::Justification::centred);
    }

    // Draw strings with thickness variations for realism
    juce::Array<float> stringThickness = {1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f};
    for (int i = 0; i < 6; ++i)  // Strings
    {
        g.setColour(juce::Colours::darkslategrey);
        g.drawLine(50, 100 + i * 30, getWidth() - 50, 100 + i * 30, stringThickness[i]);  // Adjusted vertical spacing for strings
    }

    // Add fret markers (only on the 3rd fret)
    g.setColour(juce::Colours::black);
    g.fillEllipse(160, 230, 10, 10);  // Fret marker at the 3rd fret

    // Highlight chord positions with markers between frets
    if (!currentChordPositions.empty())
    {
        g.setColour(juce::Colours::red);
        for (const auto& pos : currentChordPositions)
        {
            int string = pos.first;  // String number (1 to 6)
            int fret = pos.second;    // Fret number (0 to 4)

            // Adjust finger position between frets
            float xPos = 80 + fret * 80 - 40;  // Reduced fret spacing
            float yPos = 100 + string * 30 - 10;  // Adjusted string spacing

            // Draw a red circle for finger position
            g.fillEllipse(xPos, yPos, 15, 15);  // Smaller marker size

            // Add inner white highlight for a 3D effect
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.fillEllipse(xPos + 3, yPos + 3, 9, 9);  // Smaller inner highlight
        }
    }

    // Draw X markers for muted strings
    if (!mutedStrings.empty())
    {
        g.setColour(juce::Colours::black);
        for (int string : mutedStrings)
        {
            float xPos = 50;  // X position is fixed above the fretboard
            float yPos = 100 + string * 30 - 10;

            // Draw "X" above the muted string
            g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
            g.drawText("X", xPos - 30, yPos - 40, 20, 20, juce::Justification::centred);
        }
    }
}

void TabComponent1::loadChord()
{
    currentChordPositions.clear();
    mutedStrings.clear();

    // Update the chord positions and muted strings based on the selected chord
    switch (chordComboBox.getSelectedId())
    {
        case 1:  // C Major
            currentChordPositions = {{4, 3}, {3, 2}, {1, 1}};  // Corrected C Major
            mutedStrings = {5, 6};  // 5th and 6th strings muted
            chordLabel.setText("C Major", juce::dontSendNotification);
            break;

        case 2:  // G Major
            currentChordPositions = {{5, 3}, {4, 2}, {0, 3}};  // Corrected G Major
            mutedStrings = {};  // No muted strings
            chordLabel.setText("G Major", juce::dontSendNotification);
            break;

        case 3:  // D Major
            currentChordPositions = {{2, 2}, {1, 3}, {0, 2}};  // Corrected D Major
            mutedStrings = {4, 5, 6};  // 4th, 5th, and 6th strings muted
            chordLabel.setText("D Major", juce::dontSendNotification);
            break;

        case 4:  // A Major
            currentChordPositions = {{3, 2}, {2, 2}, {1, 2}};  // Corrected A Major
            mutedStrings = {5, 6};  // 5th and 6th strings muted
            chordLabel.setText("A Major", juce::dontSendNotification);
            break;

        case 5:  // E Major
            currentChordPositions = {{4, 2}, {3, 2}, {2, 1}};  // Corrected E Major
            mutedStrings = {};  // No muted strings
            chordLabel.setText("E Major", juce::dontSendNotification);
            break;

        case 6:  // A Minor
            currentChordPositions = {{3, 2}, {2, 2}, {1, 1}};  // Corrected A Minor
            mutedStrings = {5, 6};  // 5th and 6th strings muted
            chordLabel.setText("A Minor", juce::dontSendNotification);
            break;

        case 7:  // E Minor
            currentChordPositions = {{4, 2}, {3, 2}};  // Corrected E Minor
            mutedStrings = {};  // No muted strings
            chordLabel.setText("E Minor", juce::dontSendNotification);
            break;

        default:
            chordLabel.setText("Select a chord...", juce::dontSendNotification);
            break;
    }

    repaint();  // Update the UI to reflect the new chord positions and muted strings
}

void TabComponent1::resized()
{
    // Create a FlexBox for vertical and horizontal centering
    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
    flexBox.alignItems = juce::FlexBox::AlignItems::center;

    // Add the chord label and ComboBox to the FlexBox layout
    flexBox.items.add(juce::FlexItem(chordLabel).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(chordComboBox).withMinWidth(200).withMinHeight(30).withMargin(juce::FlexItem::Margin(10)));

    // Perform layout within the bounds of the component
    flexBox.performLayout(getLocalBounds().reduced(20));
}
