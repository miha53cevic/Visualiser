#include "app.h"

#include <bass.h>
#include <vector>
#include <fstream>

#include "libs/json.hpp"

#include "quad.h"
#include "camera.h"
#include "cube.h"

class VisualiserGL : public App
{
public:
    VisualiserGL(const char* title, int width, int height, nlohmann::json config)
        : App(title, width, height)
        , m_quad({width, height})
        , m_config(config)
        , m_cube({width, height})
    {
        m_volume = 1.0f;

        if (m_config["display"]["fullscreen"])
            Fullscreen(true);
    }
    ~VisualiserGL()
    {
        // Cleanup BASS
        BASS_Free();
    }

    void setAudioFilePath(std::string path)
    {
        m_audioFilePath = path;

        // Get the audio file title withouth the path
        m_audioTitle = "";
        for (int i = m_audioFilePath.size() - 1; i >= 0; i--)
        {
            if (m_audioFilePath[i] == '\\' || m_audioFilePath[i] == '/')
                break;
            else m_audioTitle += m_audioFilePath[i];
        }
        // Reverse the string so it's in the correct order
        std::reverse(m_audioTitle.begin(), m_audioTitle.end());
    }

private:
    virtual bool Event(SDL_Event& e) override
    {
        if (GetKey(SDL_SCANCODE_UP))
        {
            m_volume += 0.05f;
            if (m_volume >= 1.0f)
                m_volume = 1.0f;
            
            BASS_ChannelSetAttribute(m_handle, BASS_ATTRIB_VOL, m_volume);
        }
        else if (GetKey(SDL_SCANCODE_DOWN))
        {
            m_volume -= 0.05f;
            if (m_volume <= 0.0f)
                m_volume = 0.0f;

            BASS_ChannelSetAttribute(m_handle, BASS_ATTRIB_VOL, m_volume);
        }

        return true;
    }

    virtual bool Setup() override
    {
        //ShowCursor(false);
        setClearColor(0, 0, 0, 255);

        // Use -1 for the default device
        m_deviceID = m_config["bass"]["deviceID"];

        // Most modern music uses a sample rate of 44100
        m_sampleRate = m_config["bass"]["sampleRate"];

        BASS_Init(m_deviceID, m_sampleRate, 0, 0, nullptr);
        PlayAudio(m_audioFilePath, &m_handle);
        return true;
    }
    virtual bool Loop(float elapsed) override
    {
        // Check for song end
        if (BASS_ChannelIsActive(m_handle) == BASS_ACTIVE_STOPPED)
            return false;

        // Visualise
        auto peakmaxArray = calculatePeakMaxArray();
        if (m_config["visualiser2d"]["active"]) visualiser2d(peakmaxArray);
        if (m_config["visualiser3d"]["active"]) visualiser3d(peakmaxArray);
        
        // Draw info about the song and volume
        drawText(m_audioTitle.data(), 0, 0, 1, 1, 1, 1, 1);
        std::string volume = "Volume: " + std::to_string(m_volume);
        drawText(volume.data(), 0, 16, 1, 1, 1, 1, 1); // scale 1 font is size 16

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

    std::vector<float> calculatePeakMaxArray()
    {
        auto freq_bin = m_config["data"]["freq_bin"];
        std::vector<float> peakmaxArray;

        // Calculate 2^14 FFT
        const int size = 8192;
        float buffer[size];
        BASS_ChannelGetData(m_handle, &buffer, BASS_DATA_FFT16384);

        // Get only the bins that have a frequency between the values in freq_bin
        // using the formula: 
        //           freq = bin_count * sampleRate / N
        //
        // Source: https://stackoverflow.com/questions/4364823/how-do-i-obtain-the-frequencies-of-each-value-in-an-fft/4371627#4371627
        for (int i = 0; i < size; i++)
        {
            float freq = i * m_sampleRate / (size * 2);
            for (int j = 0; j < freq_bin.size() - 1; j++)
            {
                if ((freq > freq_bin[j]) && (freq <= freq_bin[j + 1]))
                    peakmaxArray.push_back(buffer[i]);
            }
        }
        return peakmaxArray;
    }

    void visualiser2d(std::vector<float> peakmaxArray)
    {
        float barWidth  = m_config["visualiser2d"]["rectWidth"];
        float barAmp    = m_config["visualiser2d"]["barAmp"];
        float circleAmp = m_config["visualiser2d"]["circleAmp"];
        float aproxAmp  = m_config["visualiser2d"]["aproxAmp"];

        // Spectrum colours
        auto barColour      = m_config["visualiser2d"]["barColour"];
        auto circleColour   = m_config["visualiser2d"]["circleColour"];

        // Draw bar spectrum
        float centerOffset = (float)(ScreenWidth() / 2) - (float)(peakmaxArray.size() * barWidth / 2);
        for (int i = 0; i < peakmaxArray.size(); i++)
        {
            m_quad.setPosition(centerOffset + i * barWidth, ScreenHeight());
            m_quad.setSize(barWidth, -peakmaxArray[i] * barAmp);
            m_quad.setColour({ barColour[0], barColour[1], barColour[2], barColour[3] });
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
            float initRadius = m_config["visualiser2d"]["circleInitialRadius"];
            float r = initRadius + aprox;

            float x = cx + r * cosf(glm::radians(a));
            float y = cy + r * sinf(glm::radians(a));

            m_quad.setPosition(x, y);
            m_quad.setSize(barWidth, -peakmaxArray[i] * circleAmp);
            m_quad.setRotation(a + 90);
            m_quad.setColour({ circleColour[0], circleColour[1], circleColour[2], circleColour[3] });
            m_quad.Draw();
        }
    }

    void visualiser3d(std::vector<float> peakmaxArray)
    {
        auto cameraPos      = m_config["visualiser3d"]["cameraPos"];
        auto cameraRot      = m_config["visualiser3d"]["cameraRot"];
        auto barAmp         = m_config["visualiser3d"]["barAmp"];
        auto barColour      = m_config["visualiser3d"]["barColour"];
        auto circleRadius   = m_config["visualiser3d"]["circleRadius"];

        m_camera.setPosition({ cameraPos[0], cameraPos[1], cameraPos[2] });
        m_camera.setRotation({ cameraRot[0], cameraRot[1], cameraRot[2] });

        // Get angle to rotate for circle points
        const float angle = 360 / peakmaxArray.size();
        for (int i = 0; i < peakmaxArray.size(); i++)
        {
            // Angle in circle
            const float a = i * angle;

            float cx = 0;
            float cy = 0;
            float r = circleRadius;

            float x = cx + r * cosf(glm::radians(a));
            float y = cy + r * sinf(glm::radians(a));

            m_cube.setScale({ 1, peakmaxArray[i] * barAmp, 1 });
            m_cube.setPosition({ x, 0, y });
            m_cube.setColour({ barColour[0], barColour[1], barColour[2], barColour[3] });
            m_cube.Draw(&m_camera);
        }
    }

private:
    HSTREAM m_handle;
    int     m_sampleRate;
    int     m_deviceID;
    float   m_volume;

    std::string     m_audioFilePath;
    std::string     m_audioTitle;
    nlohmann::json  m_config;

    // 2d
    Quad   m_quad;

    // 3d
    Camera m_camera;
    Cube   m_cube;
};

int main(int argc, char* argv[])
{
    // Try to load config.json from executable folder location
    char* basePath = SDL_GetBasePath();
    std::ifstream reader(basePath + std::string("config.json"));
    SDL_free(basePath);

    nlohmann::json config;
    if (reader.bad())
        printf("Could not find config.json!\n");
    else reader >> config;

    VisualiserGL app("Visualiser", config["display"]["width"], config["display"]["height"], config);

    if (argc >= 2)
        app.setAudioFilePath(argv[1]);

    app.Run();
    return 0;
}