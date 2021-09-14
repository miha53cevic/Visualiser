#include "app.h"

#include <bass.h>
#include <vector>
#include <fstream>
#include <list>

#include "libs/json.hpp"

#include "quad.h"
#include "camera.h"
#include "cube.h"

// C++17
#ifdef _WIN32
    #include <filesystem>
    namespace fs = std::experimental::filesystem;
#elif __linux__
    #include <filesystem>
    namespace fs = std::filesystem;
#endif

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

    void singleMode(std::string audioFilePath)
    {
        addSong(audioFilePath);
    }

    void playlistMode()
    {
        // Recursive search through current folder and then add them to list
        std::function<void(fs::path)> DisplayDirTree = [&](const fs::path& pathToShow)
        {
            if (fs::exists(pathToShow) && fs::is_directory(pathToShow))
            {
                // Entry is type directory_entry
                for (const auto& entry : fs::directory_iterator(pathToShow))
                {
                    auto filename = entry.path().filename();
                    if (fs::is_directory(entry.path()))
                    {
                        DisplayDirTree(entry.path());
                    }
                    else if (fs::is_regular_file(entry.path()))
                    {
                        // only add .mp3 files
                        if (filename.extension() == ".mp3")
                            addSong(entry.path().string());
                    }
                }
            }
        };

        DisplayDirTree(fs::current_path());
        printf("Current working dir: %s\n", fs::current_path().string().c_str());

        printf("Using playlist mode: Found %d .mp3 songs\n\n", (int)m_songList.size());
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
        else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_RIGHT)
        {
            if (!m_songList.empty())
                playNext();
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
        playNext();
        return true;
    }

    virtual bool Loop(float elapsed) override
    {
        // Check for song end
        if (BASS_ChannelIsActive(m_handle) == BASS_ACTIVE_STOPPED)
        {
            if (!m_songList.empty())
                playNext();
            // return false exists the app program
            else return false;
        }

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
    void playNext()
    {
        if (!m_songList.empty())
        {
            PlayAudio(m_songList.front(), &m_handle);

            // Get audio file title (sets the variable m_audioTitle and returns it)
            getAudioFileName(m_songList.front());

            // Remove audio file from list (queue)
            m_songList.pop_front();
        }
        else
            printf("SongList is empty!\n");
    }

    void addSong(std::string songPath)
    {
        m_songList.push_back(songPath);
    }

    void PlayAudio(std::string path, HSTREAM* streamHandle)
    {
        // Stop any previous audio from playing (also it clears the buffer by stopping it)
        BASS_ChannelStop(*streamHandle);
        BASS_StreamFree(*streamHandle); // Important! memory leak otherwise

        *streamHandle = BASS_StreamCreateFile(false, path.data(), 0, 0, 0);

        if (!BASS_ChannelPlay(*streamHandle, false))
            printf("Could not load audio file... %s\n", path.c_str());
        else
            printf("Now playing... %s\n", path.c_str());
    }

    // Returns audio file name from path and stores it in m_audioTitle as well
    std::string getAudioFileName(std::string path)
    {
        // Get the audio file title withouth the path
        m_audioTitle = "";
        for (int i = path.size() - 1; i >= 0; i--)
        {
            if (path[i] == '\\' || path[i] == '/')
                break;
            else m_audioTitle += path[i];
        }
        // Reverse the string so it's in the correct order
        std::reverse(m_audioTitle.begin(), m_audioTitle.end());

        return m_audioTitle;
    }

    glm::vec4 HSVtoRGB(float H, float S, float V) {
        if (H > 360 || H < 0 || S>100 || S < 0 || V>100 || V < 0) {
            printf("The given HSV values are not in valid range!\n");
        }
        float s = S / 100;
        float v = V / 100;
        float C = s * v;
        float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
        float m = v - C;
        float r, g, b;
        if (H >= 0 && H < 60) {
            r = C, g = X, b = 0;
        }
        else if (H >= 60 && H < 120) {
            r = X, g = C, b = 0;
        }
        else if (H >= 120 && H < 180) {
            r = 0, g = C, b = X;
        }
        else if (H >= 180 && H < 240) {
            r = 0, g = X, b = C;
        }
        else if (H >= 240 && H < 300) {
            r = X, g = 0, b = C;
        }
        else {
            r = C, g = 0, b = X;
        }
        int R = (r + m) * 255;
        int G = (g + m) * 255;
        int B = (b + m) * 255;
        
        return { R, G, B, 255 };
    }

private:
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
        const std::vector<float> cameraPos      = m_config["visualiser3d"]["cameraPos"];
        const std::vector<float> cameraRot      = m_config["visualiser3d"]["cameraRot"];
        const float barAmp                      = m_config["visualiser3d"]["barAmp"];
        const std::vector<int> barHSV           = m_config["visualiser3d"]["barHSV"];
        const float circleRadius                = m_config["visualiser3d"]["circleRadius"];
        
        m_camera.setPosition({ cameraPos[0], cameraPos[1], cameraPos[2] });
        m_camera.setRotation({ cameraRot[0], cameraRot[1], cameraRot[2] });

        // Get angle to rotate for the circle arc
        const float startAngle  = m_config["visualiser3d"]["startAngle"];
        const float endAngle    = m_config["visualiser3d"]["endAngle"];;
        const float availableAngleSpace = endAngle - startAngle;

        const float angle = availableAngleSpace / peakmaxArray.size();
        for (int i = 0; i < peakmaxArray.size(); i++)
        {
            // Angle in arc
            const float a = i * angle + startAngle;

            float cx = 0;
            float cy = 0;
            float r = circleRadius;

            float x = cx + r * cosf(glm::radians(a));
            float z = cy + r * sinf(glm::radians(a));

            m_cube.setScale({ 1, peakmaxArray[i] * barAmp, 1 });
            m_cube.setPosition({ x, 0, z });
            m_cube.setColour(HSVtoRGB(i * ((360 - barHSV[0]) / peakmaxArray.size()) + barHSV[0], barHSV[1], barHSV[2]));
            m_cube.Draw(&m_camera);
        }
    }

private:
    HSTREAM m_handle;
    int     m_sampleRate;
    int     m_deviceID;
    float   m_volume;

    std::string     m_audioTitle;
    nlohmann::json  m_config;

    // 2d
    Quad   m_quad;

    // 3d
    Camera m_camera;
    Cube   m_cube;

    std::list<std::string> m_songList;
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

    // Visualise single song given as argument
    if (argc >= 2)
        app.singleMode(argv[1]);
    // If no arguments are given open the user interface that allows the user to choose multiple songs
    else
        app.playlistMode();

    app.Run();
    return 0;
}