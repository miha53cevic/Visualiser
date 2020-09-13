#include "app.h"

#include <bass.h>
#include <vector>

#include "quad.h"

class VisualiserGL : public App
{
public:
    VisualiserGL(const char* title, int width, int height)
        : App(title, width, height)
        , m_quad({width, height})
    {}
    ~VisualiserGL()
    {
        // Cleanup BASS
        BASS_Free();
    }

    void setAudioFilePath(std::string path) { m_audioFilePath = path; }

private:
    virtual bool Event(SDL_Event& e) override
    {
        if (GetKey(SDL_SCANCODE_UP))
        {
            m_volume += 0.05f;
            if (m_volume >= 1.0f)
                m_volume = 1.0f;
            
            BASS_SetVolume(m_volume);
        }
        else if (GetKey(SDL_SCANCODE_DOWN))
        {
            m_volume -= 0.05f;
            if (m_volume <= 0.0f)
                m_volume = 0.0f;

            BASS_SetVolume(m_volume);
        }

        return true;
    }

    virtual bool Setup() override
    {
        //ShowCursor(false);
        setClearColor(0, 0, 0, 255);

        // Use default device
        m_deviceID = -1;

        // Most modern music uses a sample rate of 44100
        m_sampleRate = 44100;

        BASS_Init(m_deviceID, m_sampleRate, 0, 0, nullptr);
        PlayAudio(m_audioFilePath, &m_handle);
        return true;
    }
    virtual bool Loop(float elapsed) override
    {
        // Check for song end
        if (BASS_ChannelIsActive(m_handle) == BASS_ACTIVE_STOPPED)
            return false;

        CreateVisualisation(8, 1000.0f, 1000.0f, 5000.0f);
        return true;
    }

private:
    void PlayAudio(std::string path, HSTREAM* streamHandle)
    {
        *streamHandle = BASS_StreamCreateFile(false, path.data(), 0, 0, 0);

        if (!BASS_ChannelPlay(*streamHandle, false))
            printf("Could not load audio file... %s\n", path.c_str());
        else
            printf("Now playing... %s\n", path.c_str());
    }

    void CreateVisualisation(int rectWidth, float barAmp, float circleAmp, float aproxAmp)
    {
        std::vector<float> freq_bin = { 20, 60, 250, 500/*, 2000, 4000, 6000, nqyuist*/ };
        std::vector<float> peakmaxArray;

        const int size = 8192;
        float buffer[size];
        BASS_ChannelGetData(m_handle, &buffer, BASS_DATA_FFT16384);

        for (int i = 0; i < size; i++)
        {
            float freq = i * m_sampleRate / (size * 2);
            for (int j = 0; j < freq_bin.size() - 1; j++)
            {
                if ((freq > freq_bin[j]) && (freq <= freq_bin[j + 1]))
                    peakmaxArray.push_back(buffer[i]);
            }
        }

        // Draw bar spectrum
        for (int i = 0; i < peakmaxArray.size(); i++)
        {
            m_quad.setPosition(i * rectWidth, ScreenHeight());
            m_quad.setSize(rectWidth, -peakmaxArray[i] * barAmp);
            m_quad.setColour({ 255, 0, 0, 255 });
            m_quad.setRotation(0);
            m_quad.Draw();
        }

        // Draw circle spectrum
        float aprox = 0.0f;
        for (int i = 0; i < peakmaxArray.size(); i++)
            aprox += peakmaxArray[i] * aproxAmp;
        aprox /= peakmaxArray.size();

        // Get angle to rotate for circle points
        const float angle = 360 / peakmaxArray.size();
        for (int i = 0; i < peakmaxArray.size(); i++)
        {
            // Angle in circle
            const float a = i * angle;

            float cx = ScreenWidth()  / 2;
            float cy = ScreenHeight() / 2;
            float r = 100 + aprox;

            float x = cx + r * cosf(glm::radians(a));
            float y = cy + r * sinf(glm::radians(a));

            m_quad.setPosition(x, y);
            m_quad.setSize(rectWidth, -peakmaxArray[i] * circleAmp);
            m_quad.setRotation(a + 90);
            m_quad.setColour({ 0, 0, 255, 255 });
            m_quad.Draw();
        }
    }

private:
    HSTREAM m_handle;
    int     m_sampleRate;
    int     m_deviceID;

    Quad    m_quad;

    float   m_volume;

    std::string m_audioFilePath;
};

int main(int argc, char* argv[])
{
    VisualiserGL app("Visualiser", 1280, 720);

    if (argc >= 2)
    {
        app.setAudioFilePath(argv[1]);
    }

    app.Run();
    return 0;
}