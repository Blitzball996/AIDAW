#include "Metronome.hpp"
#include <cmath>

namespace magda
{

Metronome::Metronome()
{
    formatManager_.registerBasicFormats();
}

Metronome::~Metronome() = default;

//==============================================================================
void Metronome::setEnabled (bool enabled)    { enabled_ = enabled; }
bool Metronome::isEnabled() const noexcept   { return enabled_; }

void Metronome::setVolume (float volume)     { volume_ = juce::jlimit (0.0f, 1.0f, volume); }
float Metronome::getVolume() const noexcept  { return volume_; }

void Metronome::setAccentFirst (bool accent) { accentFirst_ = accent; }
bool Metronome::getAccentFirst() const noexcept { return accentFirst_; }

void Metronome::setCountIn (int bars)        { countInBars_ = juce::jmax (0, bars); }
int Metronome::getCountIn() const noexcept   { return countInBars_; }

bool Metronome::isCountingIn() const noexcept { return countingIn_; }

void Metronome::setSubdivision (int subdivision)
{
    if (subdivision == 1 || subdivision == 2 || subdivision == 4)
        subdivision_ = subdivision;
}

int Metronome::getSubdivision() const noexcept { return subdivision_; }

//==============================================================================
void Metronome::startCountIn()
{
    if (countInBars_ > 0)
    {
        countingIn_ = true;
        countInBeatsRemaining_ = countInBars_ * 4; // Assume 4/4 for count-in
    }
}

void Metronome::resetCountIn()
{
    countingIn_ = false;
    countInBeatsRemaining_ = 0;
}

//==============================================================================
bool Metronome::loadClickSound (const juce::File& audioFile)
{
    auto buf = loadAudioFile (audioFile);
    if (buf == nullptr)
        return false;
    clickSound_ = std::move (buf);
    return true;
}

bool Metronome::loadAccentSound (const juce::File& audioFile)
{
    auto buf = loadAudioFile (audioFile);
    if (buf == nullptr)
        return false;
    accentSound_ = std::move (buf);
    return true;
}

void Metronome::resetToDefaultSounds()
{
    clickSound_.reset();
    accentSound_.reset();
}

//==============================================================================
void Metronome::prepareToPlay (double sampleRate, int blockSize)
{
    sampleRate_ = sampleRate;
    blockSize_  = blockSize;
    lastTriggeredBeat_ = -1.0;
    clickPlaybackPos_  = -1;
    accentPlaybackPos_ = -1;

    generateDefaultSounds();
}

//==============================================================================
void Metronome::generateClickBuffer (juce::AudioBuffer<float>& buffer,
                                     int numSamples,
                                     double tempo,
                                     int timeSignatureNumerator,
                                     double positionInBeats)
{
    if (! enabled_ || tempo <= 0.0)
        return;

    const double beatsPerSample = tempo / (60.0 * sampleRate_);
    const double subdivisionInterval = 1.0 / (double) subdivision_;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        double currentBeat = positionInBeats + sample * beatsPerSample;

        // Quantise to subdivision grid
        double quantisedBeat = std::floor (currentBeat / subdivisionInterval) * subdivisionInterval;

        // Check if we've crossed a subdivision boundary
        double prevBeat = currentBeat - beatsPerSample;
        double prevQuantised = std::floor (prevBeat / subdivisionInterval) * subdivisionInterval;

        if (quantisedBeat > prevQuantised && quantisedBeat != lastTriggeredBeat_)
        {
            lastTriggeredBeat_ = quantisedBeat;

            // Determine if this is beat 1 of a bar
            int beatInBar = ((int) std::floor (quantisedBeat)) % timeSignatureNumerator;
            bool isAccent = accentFirst_ && (beatInBar == 0)
                            && (std::fmod (quantisedBeat, 1.0) < 0.001);

            if (isAccent)
            {
                accentPlaybackPos_ = 0;
                playingAccent_ = true;
            }
            else
            {
                clickPlaybackPos_ = 0;
                playingAccent_ = false;
            }
        }

        // Mix click audio into output
        if (clickPlaybackPos_ >= 0)
        {
            auto* clickBuf = getClickBuffer();
            if (clickBuf != nullptr && clickPlaybackPos_ < clickBuf->getNumSamples())
            {
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    int srcCh = juce::jmin (ch, clickBuf->getNumChannels() - 1);
                    buffer.addSample (ch, sample,
                        clickBuf->getSample (srcCh, clickPlaybackPos_) * volume_);
                }
                ++clickPlaybackPos_;
            }
            else
            {
                clickPlaybackPos_ = -1;
            }
        }

        if (accentPlaybackPos_ >= 0)
        {
            auto* accentBuf = getAccentBuffer();
            if (accentBuf != nullptr && accentPlaybackPos_ < accentBuf->getNumSamples())
            {
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    int srcCh = juce::jmin (ch, accentBuf->getNumChannels() - 1);
                    buffer.addSample (ch, sample,
                        accentBuf->getSample (srcCh, accentPlaybackPos_) * volume_);
                }
                ++accentPlaybackPos_;
            }
            else
            {
                accentPlaybackPos_ = -1;
            }
        }
    }
}

//==============================================================================
void Metronome::generateDefaultSounds()
{
    // Generate a short sine-wave click (about 20ms)
    int clickLen = (int) (sampleRate_ * 0.02);
    defaultClick_ = std::make_unique<juce::AudioBuffer<float>> (1, clickLen);
    defaultAccent_ = std::make_unique<juce::AudioBuffer<float>> (1, clickLen);

    auto* clickData  = defaultClick_->getWritePointer (0);
    auto* accentData = defaultAccent_->getWritePointer (0);

    for (int i = 0; i < clickLen; ++i)
    {
        double t = (double) i / sampleRate_;
        double envelope = 1.0 - ((double) i / (double) clickLen); // Linear decay
        envelope *= envelope; // Exponential-ish decay

        // Normal click: 1000 Hz sine
        clickData[i] = (float) (std::sin (2.0 * juce::MathConstants<double>::pi * 1000.0 * t)
                                * envelope * 0.7);

        // Accent click: 1500 Hz sine, louder
        accentData[i] = (float) (std::sin (2.0 * juce::MathConstants<double>::pi * 1500.0 * t)
                                 * envelope * 1.0);
    }
}

//==============================================================================
const juce::AudioBuffer<float>* Metronome::getClickBuffer() const
{
    if (clickSound_ != nullptr)
        return clickSound_.get();
    return defaultClick_.get();
}

const juce::AudioBuffer<float>* Metronome::getAccentBuffer() const
{
    if (accentSound_ != nullptr)
        return accentSound_.get();
    return defaultAccent_.get();
}

//==============================================================================
std::unique_ptr<juce::AudioBuffer<float>> Metronome::loadAudioFile (const juce::File& file)
{
    if (! file.existsAsFile())
        return nullptr;

    std::unique_ptr<juce::AudioFormatReader> reader (
        formatManager_.createReaderFor (file));

    if (reader == nullptr)
        return nullptr;

    auto buffer = std::make_unique<juce::AudioBuffer<float>> (
        (int) reader->numChannels, (int) reader->lengthInSamples);

    reader->read (buffer.get(), 0, (int) reader->lengthInSamples, 0, true, true);
    return buffer;
}

} // namespace magda
