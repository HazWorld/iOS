#include "TabComponent1.hpp"


//TabComponent1 implementation
TabComponent1::TabComponent1()
{
    //UI setup
    addAndMakeVisible(chordLabel);
    chordLabel.setText("Select a chord...", juce::dontSendNotification);
    chordLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    chordLabel.setJustificationType(juce::Justification::centred);
    chordLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(infoButton);
    infoButton.setButtonText("Info");
    infoButton.onClick = [this]() { toggleInfoOverlay(); };
    addAndMakeVisible(infoOverlay);
    infoOverlay.setVisible(false);

    //combobox for dropdown menuu
    chordComboBox.addItem("C Major", 1);
    chordComboBox.addItem("G Major", 2);
    chordComboBox.addItem("D Major", 3);
    chordComboBox.addItem("A Major", 4);
    chordComboBox.addItem("E Major", 5);
    chordComboBox.addItem("A Minor", 6);
    chordComboBox.addItem("E Minor", 7);

    //loads the chord when selected
    chordComboBox.onChange = [this]() { loadChord(); };
    addAndMakeVisible(chordComboBox);

    setSize(500, 300);
}

//this handles the drawing of the frets and the placement
//of finger positions and muted strings
void TabComponent1::paint(juce::Graphics& g)
{

    g.fillAll(juce::Colour::fromRGB(240, 230, 200));

    //drawing fret lines
    juce::ColourGradient fretGradient(juce::Colours::lightgrey, 0, 0, juce::Colours::darkgrey, getWidth(), 0, false);
    g.setGradientFill(fretGradient);

   //currently limited to 4 frets, this can be expanded
    for (int i = 0; i < 4; ++i)
    {
        //fret spaceing
        int fretX = 80 + i * 80;
        g.setColour(juce::Colours::darkgrey);

        //increased first line thickness to show start of fretboard
        if (i == 0)
        {
            g.drawLine(fretX, 80, fretX, 260, 9.0f);
        }
        else
        {
            g.drawLine(fretX, 80, fretX, 260, 3.0f);
        }

        //adds fret numbers just above the fret lines
        g.setColour(juce::Colours::black);
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        g.drawText(juce::String(i + 1), fretX - 5, 60, 20, 20, juce::Justification::centred);
    }

    //draws strings with thickness differences
    juce::Array<float> stringThickness = {1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f};
    for (int i = 0; i < 6; ++i)
    {
        g.setColour(juce::Colours::darkslategrey);
        g.drawLine(50, 100 + i * 30, getWidth() - 50, 100 + i * 30, stringThickness[i]);
    }

    //adds red dots for finger positions
    if (!currentChordPositions.empty())
    {
        for (int i = 0; i < currentChordPositions.size(); ++i)
        {
            //assigns values from loadchord to the string and fret positions
            const auto& pos = currentChordPositions[i];
            int string = pos.first;
            int fret = pos.second;

            //fret and string psoitions
            float xPos = 80 + fret * 80 - 40;
            float yPos = 100 + string * 30 - 10;

            g.setColour(juce::Colours::red);
            
           
            g.fillEllipse(xPos, yPos, 15, 15);

           
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.fillEllipse(xPos + 3, yPos + 3, 9, 9);
        }
    }

   //muted strings implementation
    if (!mutedStrings.empty())
    {
        g.setColour(juce::Colours::black);
        for (int string : mutedStrings)
        {
            float xPos = 50;
            float yPos = 100 + string * 30;

            //places x beside muted strings
            g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
            g.drawText("X", xPos - 30, yPos - 40, 20, 20, juce::Justification::centred);
        }
    }
}


//chord finger positions and muted strings are stored here, add more here
void TabComponent1::loadChord()
{
    currentChordPositions.clear();
    mutedStrings.clear();

    switch (chordComboBox.getSelectedId())
    {
        case 1:
            currentChordPositions = {{4, 3}, {3, 2}, {1, 1}};
            mutedStrings = {6};
            chordLabel.setText("C Major", juce::dontSendNotification);
            break;

        case 2:
            currentChordPositions = {{5, 3}, {4, 2}, {0, 3}};
            mutedStrings = {};
            chordLabel.setText("G Major", juce::dontSendNotification);
            break;

        case 3:
            currentChordPositions = {{2, 2}, {1, 3}, {0, 2}};
            mutedStrings = {5, 6};
            chordLabel.setText("D Major", juce::dontSendNotification);
            break;

        case 4:
            currentChordPositions = {{3, 2}, {2, 2}, {1, 2}};
            mutedStrings = {6};
            chordLabel.setText("A Major", juce::dontSendNotification);
            break;

        case 5:
            currentChordPositions = {{4, 2}, {3, 2}, {2, 1}};
            mutedStrings = {};
            chordLabel.setText("E Major", juce::dontSendNotification);
            break;

        case 6:
            currentChordPositions = {{3, 2}, {2, 2}, {1, 1}};
            mutedStrings = {6};
            chordLabel.setText("A Minor", juce::dontSendNotification);
            break;

        case 7:
            currentChordPositions = {{4, 2}, {3, 2}};
            mutedStrings = {};
            chordLabel.setText("E Minor", juce::dontSendNotification);
            break;

        default:
            chordLabel.setText("Select a chord...", juce::dontSendNotification);
            break;
    }

    repaint();
}


//UI layout
void TabComponent1::resized()
{
    
    auto bounds = getLocalBounds().reduced(20);

  
    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
    flexBox.alignItems = juce::FlexBox::AlignItems::center;

    flexBox.items.add(juce::FlexItem(chordLabel).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(chordComboBox).withMinWidth(200).withMinHeight(30).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(infoButton).withMinWidth(150).withMinHeight(30).withMargin(juce::FlexItem::Margin(10)));

    flexBox.performLayout(bounds);

    infoOverlay.setBounds(getLocalBounds());
}


//info overlay
void TabComponent1::toggleInfoOverlay()
{
    juce::MessageManager::callAsync([this]()
    {
        if (!infoOverlay.isVisible())
        {
            infoOverlay.setInfoContent("Welcome to the chord library!!\n\nSelect a chord from the list to learn\nhow to play it!!\n\nFinger positions are shown in RED\n\nMuted Strings (you shouldnt play these) are marked with an X");
            infoOverlay.setVisible(true);
            infoOverlay.toFront(true);
        }
        else
        {
            infoOverlay.setVisible(false);
        }
    });
}
