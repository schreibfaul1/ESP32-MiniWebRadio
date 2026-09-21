#include "Audio.h"
#include "function.h"

#pragma once

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  S D _ C O N T E N T   📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class SD_content {

  private:
    struct FileInfo {
        int32_t      fileSize;
        ps_ptr<char> fileName;
        ps_ptr<char> filePath;

        FileInfo(int32_t fs, ps_ptr<char> fn, ps_ptr<char> fp) : fileSize(fs), fileName(fn), filePath(fp) {}

        ~FileInfo() = default;
        FileInfo(const FileInfo&) = default;
        FileInfo& operator=(const FileInfo&) = default;
    };
    std::vector<FileInfo> m_files;
    ps_ptr<char>          m_buff;
    ps_ptr<char>          m_lastConnectedFile = "";
    ps_ptr<char>          m_lastConnectedFolder = "";
    ps_ptr<char>          m_lastConnectedFileName = "";
    ps_ptr<char>          m_JSONstr;

  public:
    SD_content() { m_files.clear(); }
    ~SD_content() {
        m_files.clear();
        m_JSONstr.reset();
        m_lastConnectedFile.reset();
        m_lastConnectedFolder.reset();
        m_lastConnectedFileName.reset();
    }

    bool listFilesInDir(ps_ptr<char> path, boolean audioFilesOnly, boolean withoutDirs) {
        m_files.clear();

        if (!SD_MMC.exists(path.c_get())) {
            printfln(s_tag.sd_card, ANSI_ESC_RED "SD_MMC/{} not exist", path);
            return false;
        }

        File masterFile = SD_MMC.open(path.c_get());

        if (!masterFile || !masterFile.isDirectory()) {
            printfln(s_tag.sd_card, ANSI_ESC_RED "SD_MMC/{} is not a directory", path);
            masterFile.close();
            return false;
        }

        while (true) {
            File slaveFile = masterFile.openNextFile();
            if (!slaveFile) break;

            if (slaveFile.isDirectory()) {
                if (!withoutDirs) {
                    ps_ptr<char> filePath = slaveFile.path();
                    filePath.append("/");

                    m_files.emplace_back(-1, slaveFile.name(), filePath);
                }
            } else {
                ps_ptr<char> name = slaveFile.name();
                if (!audioFilesOnly ||         //
                    name.ends_with(".mp3") ||  //
                    name.ends_with(".aac") ||  //
                    name.ends_with(".m4a") ||  //
                    name.ends_with(".wav") ||  //
                    name.ends_with(".m3u") ||  //
                    name.ends_with(".flac") || //
                    name.ends_with(".opus") || //
                    name.ends_with(".ogg")) {  //

                    m_files.emplace_back( //
                        slaveFile.size(), //
                        slaveFile.name(), //
                        slaveFile.path()  //
                    );
                }
            }
            slaveFile.close();
        }
        sort();
        masterFile.close();
        return true;
    }

    bool isDir(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_ERROR("idx {} is oor, max = {}", idx, m_files.size() - 1);
            return false;
        }

        return m_files[idx].fileSize == -1; // -1 means dir
    }

    size_t getSize() { return m_files.size(); }

    ps_ptr<char> getColouredSStringByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            MWR_LOG_WARN("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            MWR_LOG_WARN("idx {} is oor, max = {}", idx, m_files.size());
            return "";
        }
        if (m_files[idx].fileSize == -1) return m_files[idx].fileName;                            // directory
        m_buff.assignf("{}" ANSI_ESC_YELLOW " {}", m_files[idx].fileName, m_files[idx].fileSize); // file
        return m_buff;
    }

    ps_ptr<char> getFileNameByIndex(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_WARN("idx {} is oor, size = {}", idx, m_files.size());
            return "";
        }
        return m_files[idx].fileName;
    }

    int32_t getFileSizeByIndex(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_WARN("idx {} is oor, size = {}", idx, m_files.size());
            return 0;
        }
        return m_files[idx].fileSize; // returns -1 for dirs
    }

    ps_ptr<char> getFilePathByIndex(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_WARN("idx {} is oor, size = {}", idx, m_files.size());
            return "";
        }
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getFilePathByIndex(0) returns "/dir_a/"
            getFilePathByIndex(3) returns "/dir_a/dir_b/file_b"
            getFilePathByIndex(5) returns "/dir_a/file_d"
        */
        return m_files[idx].filePath;
    }

    ps_ptr<char> getFileFolderByIndex(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_WARN("idx {} is oor, size = {}", idx, m_files.size());
            return "";
        }
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getFileFolderByIndex(0) returns "/dir_a/"
            getFileFolderByIndex(1) returns "/dir_a/dir_b/"
            getFileFolderByIndex(5) returns "/dir_a/"
        */
        if (isDir(idx)) return m_files[idx].filePath;
        int lastSlashIndex = m_files[idx].filePath.last_index_of('/');
        m_buff = m_files[idx].filePath;
        m_buff[lastSlashIndex + 1] = '\0';
        return m_buff;
    }

    int16_t getIndexByName(ps_ptr<char> path) {
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getIndexByName("/dir_a") returns 0
            getIndexByName("/dir_a/dir_b/file_b") returns 3
            getIndexByName("/dir_a/dir_b/file_y") returns -1
        */
        if (!path.valid()) return -1;
        for (int i = 0; i < m_files.size(); i++) {
            if (m_files[i].filePath.equals(path)) { return i; }
        }
        return -1;
    }

    uint16_t getNextAudioFile(uint16_t currIdx) { // assume listFilesInDir with "audioFilesOnly"
        if (m_files.empty()) return 0;
        if (currIdx >= m_files.size()) currIdx = m_files.size() - 1;
        uint16_t newIdx = currIdx;
        while (true) {
            newIdx++;
            if (newIdx >= m_files.size()) newIdx = 0;
            if (newIdx == currIdx) break;                           // avoid an infinite loop
            if (!m_files[newIdx].fileName.ends_with(".m3u")) break; // skip m3u files
        }
        return newIdx;
    }

    uint16_t getPrevAudioFile(uint16_t currIdx) { // assume listFilesInDir with "audioFilesOnly"
        if (m_files.empty()) return 0;
        if (currIdx >= m_files.size()) currIdx = m_files.size() - 1;
        uint16_t newIdx = currIdx;
        while (true) {
            if (newIdx == 0) {
                newIdx = m_files.size() - 1;
            } else {
                --newIdx;
            }
            if (newIdx == currIdx) break;                           // avoid an infinite loop
            if (!m_files[newIdx].fileName.ends_with(".m3u")) break; // skip m3u files
        }
        return newIdx;
    }

    void setLastConnectedFile(ps_ptr<char> lastconnectedItem) {
        /*  lastconnectedItem                       m_lastConnectedFolder       m_lastConnectedFileName     m_lastConnectedFile
            "xyz/chicken.wav"                       "/audiofiles/"              {}                          "/audiofiles/"                      // does not start with "/"
            "/audiofiles/wavfiles/chickenwav"       "/audiofiles/wavfiles/"     {}                          "/audiofiles/wavfiles/"             // file has no extension
            "/audiofiles/wavfiles/.wav"             "/audiofiles/wavfiles/"     {}                          "/audiofiles/wavfiles/"             // file has no name
            "/chicken.wav"                          "/"                         "chicken.wav"               "/chicken.wav"                      // we have no folder
            "/audiofiles/wavfiles/"                 "/audiofiles/wavfiles/"     {}                          "/audiofiles/wavfiles/"             // we have no file
            "/audiofiles/wavfiles/chicken.wav"      "/audiofiles/wavfiles/"     "chicken.wav"               "/audiofiles/wavfiles/chicken.wav"
        */
        int posFirst = 0, posLast = 0, posDot = 0;

        if (!lastconnectedItem.valid()) { goto exit; } // guard, lastconnectedItem == NULL

        posFirst = lastconnectedItem.index_of("/", 0);
        if (posFirst != 0) { // guard, does not start with /
            m_lastConnectedFolder.assign("/audiofiles/");
            m_lastConnectedFileName.reset();
            goto exit;
        }

        posLast = lastconnectedItem.last_index_of('/');
        if (posFirst == posLast) { // we have no folder
            m_lastConnectedFolder = "/";
        } else {
            m_lastConnectedFolder = lastconnectedItem.substr(0, posLast + 1);
        }

        posDot = lastconnectedItem.index_of('.', posLast);
        if (posDot == -1) { // file has no extension
            m_lastConnectedFileName.reset();
        } else {
            if (posDot == posLast + 1) {
                m_lastConnectedFileName.reset(); // extension without name
            }
            m_lastConnectedFileName = lastconnectedItem.substr(posLast + 1); // fileNane exists
        }

    exit:
        m_lastConnectedFile.clone_from(m_lastConnectedFolder);
        m_lastConnectedFile.append(m_lastConnectedFileName);
        MWR_LOG_DEBUG("lastconnectedItem {}", lastconnectedItem);
        MWR_LOG_DEBUG("m_lastConnectedFolder {}", m_lastConnectedFolder);
        MWR_LOG_DEBUG("m_lastConnectedFileName {}", m_lastConnectedFileName);
        MWR_LOG_DEBUG("lastConnectedFile {}", m_lastConnectedFile);

        listFilesInDir(m_lastConnectedFolder, true, false);
    }

    ps_ptr<char> getLastConnectedFolder() { return m_lastConnectedFolder; }
    ps_ptr<char> getLastConnectedFileName() { return m_lastConnectedFileName; }

    int16_t getPosByFileName(ps_ptr<char> fileName) {
        for (size_t i = 0; i < m_files.size(); i++) {
            if (m_files[i].fileName == fileName) return i; // fileName e.g. "file.mp3"
        }
        return -1;
    }

    ps_ptr<char> stringifyDirContent(ps_ptr<char> path) {
        /*
                Music/
                Pictures/
                song.mp3
                test.flac

                [
                  {"name":"Music","dir":true,"size":0},
                  {"name":"Pictures","dir":true,"size":0},
                  {"name":"song.mp3","dir":false,"size":123456},
                  {"name":"test.flac","dir":false,"size":654321}
                ]
        */

        if (!listFilesInDir(path, false, false)) return "[]";
        m_JSONstr.assign("[");
        bool first = true;
        for (size_t i = 0; i < m_files.size(); ++i) {
            if (m_files[i].fileName.starts_with(".")) continue;
            if (!first) m_JSONstr.append(",");
            first = false;
            m_JSONstr.append("{\"name\":\"");
            m_JSONstr.append(m_files[i].fileName);
            m_JSONstr.append("\",\"dir\":");
            if (isDir(i)) {
                m_JSONstr.append("true,\"size\":0");
            } else {
                m_JSONstr.appendf("false,\"size\":{}", m_files[i].fileSize);
            }
            m_JSONstr.append("}");
        }
        m_JSONstr.append("]");
        return m_JSONstr;
    }

  private:
    void sort() {
        std::sort(m_files.begin(), m_files.end(), [](const FileInfo& a, const FileInfo& b) {
            // Zuerst nach Ordner vs. Datei sortieren
            if (a.fileSize == -1 && b.fileSize != -1) {
                return true; // a ist Ordner, b ist Datei
            }
            if (a.fileSize != -1 && b.fileSize == -1) {
                return false; // a ist Datei, b ist Ordner
            }
            // Wenn beide entweder Ordner oder beide Dateien sind, alphabetisch sortieren
            return strcmp(a.fileName.get(), b.fileName.get()) < 0;
        });
    }
};

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  S T A T I O N S M A N A G E M E N T    📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class stationManagement {
  private:
    struct sta {
        std::vector<uint8_t>      fav;
        std::vector<uint16_t>     favStaNr;
        std::vector<ps_ptr<char>> country;
        std::vector<ps_ptr<char>> name;
        std::vector<ps_ptr<char>> url;
    } m_stations;

    uint16_t  m_staCnt = 0;
    uint16_t  m_staFavCnt = 0;
    uint16_t* m_curStation = 0;

  public:
    stationManagement(uint16_t* curStation) {
        clearStations();
        m_curStation = curStation;
    }
    ~stationManagement() { clearStations(); }

  private:
    void clearStations() {
        m_stations.country.clear();
        m_stations.name.clear();
        m_stations.url.clear();
        m_stations.fav.clear();
        m_stations.fav.shrink_to_fit();
        m_stations.favStaNr.clear();
        m_stations.favStaNr.shrink_to_fit();

        m_stations.country.push_back("unknown");
        m_stations.name.push_back("unknown");
        m_stations.url.push_back("unknown");
        m_stations.fav.push_back('0');
        m_stations.favStaNr.push_back(0);
    }

  public:
    void updateStationsList() {
        clearStations();
        uint8_t item = 0;
        m_staCnt = 0;
        m_staFavCnt = 0;
        if (!SD_MMC.exists("/stations.json")) { return; }
        char buff[1024];
        File file = SD_MMC.open("/stations.json");
        while (file.available()) {
            char c = file.read();
            if (c == '[' || c == ']' || c == ',' || c == '\n' || c == '\r') { continue; } // skip
            if (c == '\"') {                                                              // start of string
                int pos = file.readBytesUntil('\"', buff, 1024);
                buff[pos] = 0;

                if (item == 0) {
                    m_stations.fav.push_back(buff[0]);
                    m_staCnt++;
                    if (buff[0] == '*') {
                        m_staFavCnt++;
                        m_stations.favStaNr.push_back(m_staCnt);
                    }
                }
                if (item == 1) { m_stations.country.push_back(buff); }
                if (item == 2) { m_stations.name.push_back(buff); }
                if (item == 3) { m_stations.url.push_back(buff); }
                item++;
                if (item > 3) item = 0;
                if (m_staCnt > 999) break;
            }
        }
        file.close();
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t getCurrentStationNumber() { return *m_curStation; }
    //----------------------------------------------------------------------------------------------------------
    uint16_t getSumStations() { return m_staCnt; }
    //----------------------------------------------------------------------------------------------------------
    uint16_t getSumFavStations() { return m_staFavCnt; }
    //----------------------------------------------------------------------------------------------------------
    uint16_t nextStation() {
        if (!m_staCnt) return 1;
        (*m_curStation)++;
        if (*m_curStation > m_staCnt) *m_curStation = 1;
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t nextFavStation() {
        if (!m_staCnt) return 1;
        uint16_t cnt = 0;
        int16_t  tmp = (*m_curStation);
        while (true) {
            tmp++;
            cnt++;
            if (cnt > m_staCnt) break;
            if (tmp > m_staCnt) tmp = 1;
            if (m_stations.fav[tmp] == '*') {
                *m_curStation = tmp;
                break;
            }
        }
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t prevStation() {
        if (!m_staCnt) return 1;
        (*m_curStation)--;
        if (*m_curStation < 1) *m_curStation = m_staCnt;
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t prevFavStation() {
        if (!m_staCnt) return 1;
        uint16_t cnt = 0;
        int16_t  tmp = (*m_curStation);
        while (true) {
            tmp--;
            cnt++;
            if (cnt > m_staCnt) break;
            if (tmp < 1) tmp = m_staCnt;
            if (m_stations.fav[tmp] == '*') {
                *m_curStation = tmp;
                break;
            }
        }
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t setStationByNumber(uint16_t staNr) {
        if (!m_staCnt) return 1;
        if (staNr > m_staCnt)
            *m_curStation = m_staCnt;
        else if (staNr == 0) {
            *m_curStation = 1;
        } else {
            *m_curStation = staNr;
        }
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    ps_ptr<char> getStationName(uint16_t staNr) {
        if (staNr > m_staCnt) return {};
        if (!m_stations.name[staNr]) return {};
        return m_stations.name[staNr];
    }
    char getStationFav(uint16_t staNr) { // 0 = not fav, * = fav, 1..3 = fav number (notused)
        if (staNr > m_staCnt) return '0';
        if (!m_stations.fav[staNr]) return '0';
        return m_stations.fav[staNr];
    }
    ps_ptr<char> getStationUrl(uint16_t staNr) {
        if (staNr > m_staCnt) return "unknown";
        if (!m_stations.url[staNr]) return "unknown";
        return m_stations.url[staNr];
    }
    ps_ptr<char> getStationCountry(uint16_t staNr) {
        if (staNr > m_staCnt) return strdup("unknown");
        if (!m_stations.country[staNr].valid()) return "unknown";
        return m_stations.country[staNr];
    }
};

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌   P L A Y L I S T     📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Playlist {

  public:
    Playlist() {}
    ~Playlist() {}

  private:
    ps_ptr<char>        m_playlist_path = {};
    deque<ps_ptr<char>> m_content_file = {};
    deque<ps_ptr<char>> m_content_items = {};
    File                m_playlist_file;
    int16_t             m_index = -1;

    void reset() {
        m_playlist_path.clear();
        m_content_file.clear();
        m_content_items.clear();
        m_playlist_file.close();
        m_index = -1;
    }

    boolean isAudio(ps_ptr<char> file) {
        if (file.ends_with(".mp3") ||  //
            file.ends_with(".aac") ||  //
            file.ends_with(".m4a") ||  //
            file.ends_with(".wav") ||  //
            file.ends_with(".flac") || //
            file.ends_with(".opus") || //
            file.ends_with(".ogg")) {  //
            return true;
        }
        return false;
    }

  public:
    bool create_playlist_from_file(ps_ptr<char> path) {
        reset();

        if (!path.valid()) return false;

        if (!path.ends_with(".m3u")) {
            MWR_LOG_ERROR("wrong playlist path {}", path);
            return false;
        }

        if (!SD_MMC.exists(path.get())) {
            MWR_LOG_ERROR("Playlistfile {} not found", path);
            return false;
        }

        m_playlist_file = SD_MMC.open(path.get());

        if (m_playlist_file.size() > 1048576) {
            MWR_LOG_ERROR("Playlist too big, size is {}", m_playlist_file.size());
            reset();
            return false;
        }

        int idx = path.last_index_of('/');
        if (idx != 0) {
            m_playlist_path = path.substr(0, idx + 1);
        } else {
            m_playlist_path = path;
        }
        ps_ptr<char> readBuff;
        ps_ptr<char> itemName;
        ps_ptr<char> itemPath;

        readBuff.alloc(2024);
        itemName.alloc(2024);
        itemPath.alloc(2024);

        bool hasExtInfo = false;

        while (m_playlist_file.available()) {

            size_t bytesRead = m_playlist_file.readBytesUntil('\n', readBuff.get(), readBuff.size());

            if (bytesRead < 1) continue;

            readBuff[bytesRead] = '\0';
            readBuff.trim();

            if (readBuff.empty()) continue;                // blank line
            if (readBuff.starts_with("#EXTM3U")) continue; // #EXTM3U

            // #EXTINF
            if (readBuff.starts_with("#EXTINF:")) {
                itemName = readBuff.substr(8);
                int comma = itemName.index_of(',');
                if (comma >= 0) {
                    auto duration = itemName.substr(0, comma);
                    int  d = duration.to_int32();
                    itemName = itemName.substr(comma + 1);                                               // title after the comma
                    if (d > 0) { itemName.appendf(" " ANSI_ESC_YELLOW "{}s" ANSI_ESC_RESET, duration); } // Optionally append duration
                }
                hasExtInfo = true;
                continue;
            }

            // Ignore other comments
            if (readBuff.starts_with("#")) continue;

            // -----------------------------------------
            // Here’s a proper playlist entry
            // -----------------------------------------

            if (readBuff.starts_with("file://")) {
                itemPath = readBuff.substr(7);
                itemPath.urldecode();
            } else if (readBuff.starts_with_icase("http://") || readBuff.starts_with_icase("https://")) {
                itemPath = readBuff;
            } else {
                if (!readBuff.starts_with("/")) {
                    itemPath = m_playlist_path + readBuff;
                } else {
                    itemPath = readBuff;
                }
                itemPath.urldecode();
            }

            // Import audio files only
            if (!isAudio(itemPath)) continue;

            // No EXTINF available:
            // Use the file name as the title
            if (!hasExtInfo) {
                int slash = itemPath.last_index_of('/');
                if (slash >= 0) {
                    itemName = itemPath.substr(slash + 1);
                } else {
                    itemName = itemPath;
                }
            }
            m_content_items.push_back(itemName);
            m_content_file.push_back(itemPath);

            // EXTINF applies only to the next entry exactly
            hasExtInfo = false;
            itemName.clear();
            itemPath.clear();
        }
        m_playlist_file.close();
        return true;
    }

    bool create_playlist_from_SD_folder(ps_ptr<char> path) { // all files within a SD folder
        reset();
        if (!SD_MMC.exists(path.get())) {
            MWR_LOG_ERROR("SD_MMC/{} not exist", path);
            return false;
        }
        File folder = SD_MMC.open(path.get());
        if (!folder.isDirectory()) {
            MWR_LOG_ERROR("SD_MMC{} is not a directory", path);
            folder.close();
            return false;
        }
        m_content_file.clear();  // clear path first
        m_content_items.clear(); // clear name first

        while (true) { // get content
            File file = folder.openNextFile();
            if (!file) break;
            if (file.isDirectory()) continue;
            if (isAudio(file.name())) {
                m_content_file.push_back(file.path());
                ps_ptr<char> name;
                name = file.name();
                name.appendf("" ANSI_ESC_YELLOW " {}" ANSI_ESC_RESET "", file.size());
                m_content_items.push_back(name);
            }
            file.close();
        }
        folder.close();

        // for (int i = 0; i < m_content_file.size(); i++) {
        //     MWR_LOG_WARN("{}, {}", i, m_content_file[i]);  // path
        //     MWR_LOG_INFO("{}, {}", i, m_content_items[i]); // name
        // }
        return true;
    }

    bool create_playlist_from_DLNA_folder(const std::deque<DLNA_Client::srvItem>* foldercontent) {
        reset();
        for (int i = 0; i < foldercontent->size(); i++) {
            MWR_LOG_DEBUG("{} : ({}) {} {} {}", i, foldercontent->at(i).isAudio, foldercontent->at(i).itemURL, foldercontent->at(i).title, foldercontent->at(i).duration);
            if (!foldercontent->at(i).isAudio) continue;
            uint16_t     itemURL_len = foldercontent->at(i).itemURL.strlen();
            uint16_t     title_len = foldercontent->at(i).title.strlen();
            uint16_t     duration_len = foldercontent->at(i).duration.strlen();
            uint16_t     len = itemURL_len + title_len + duration_len + 3;
            ps_ptr<char> itstr(len);
            itstr = foldercontent->at(i).itemURL;
            itstr += "\n";
            itstr += foldercontent->at(i).duration;
            itstr += ",";
            itstr += foldercontent->at(i).title;
            MWR_LOG_DEBUG("pushing to playlist : {}", itstr);
            m_content_file.push_back(itstr);
        }
        if (!m_content_file.size()) return false;
        MWR_LOG_INFO("pls length {}", m_content_file.size());
        return true;
    }

    void sort_alphabetical() {
        for (size_t i = 0; i < m_content_file.size(); i++) {
            for (size_t j = 1; j < m_content_file.size() - i; j++) {
                if (m_content_file[j - 1] > m_content_file[j]) {
                    swap(m_content_file[j - 1], m_content_file[j]);
                    swap(m_content_items[j - 1], m_content_items[j]);
                }
            }
        }
    }

    void sort_random() {
        if (m_content_file.size() < 2) return;

        for (size_t i = 0; i < m_content_file.size(); i++) {
            size_t randIndex = random(0, m_content_file.size());

            m_content_file[i].swap(m_content_file[randIndex]);
            m_content_items[i].swap(m_content_items[randIndex]);
        }
    }

    int16_t next_index() {
        if ((m_index + 1) == m_content_file.size()) return -1;
        m_index++;
        return m_index;
    }

    int16_t previous_index() {
        if (m_index == -1) return -1;
        m_index--;
        return m_index;
    }

    ps_ptr<char> get_file_by_index(uint16_t idx) {
        ps_ptr<char> s = "";
        if (idx < m_content_file.size()) s = m_content_file[idx];
        return s;
    }

    ps_ptr<char> get_items_by_index(uint16_t idx) {
        ps_ptr<char> s = "";
        if (idx < m_content_items.size()) s = m_content_items[idx];
        return s;
    }

    ps_ptr<char> get_file() {
        ps_ptr<char> s = "";
        if (m_index == -1) { return s; }
        if (m_index >= m_content_file.size()) { return s; }
        s = m_content_file[m_index];
        return s;
    }

    ps_ptr<char> get_items() {
        ps_ptr<char> s = "";
        if (m_index == -1) { return s; }
        if (m_index >= m_content_items.size()) { return s; }
        s = m_content_items[m_index];
        return s;
    }

    ps_ptr<char> get_coloured_file() {
        ps_ptr<char> s = "";
        if (m_index != -1) s.assignf(ANSI_ESC_CYAN "{}" ANSI_ESC_RESET, m_content_file[m_index]);
        s.println();
        return s;
    }

    ps_ptr<char> get_coloured_index() {
        ps_ptr<char> s = "";
        if (m_index != -1) s.assignf(ANSI_ESC_ORANGE "{:03}/{:03}" ANSI_ESC_RESET, m_index + 1, m_content_file.size());
        return s;
    }

    uint16_t get_size() { return m_content_file.size(); }
};

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌   R E C O R D E R     📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

struct WAVHeader {
    char     riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t size;
    char     wave[4] = {'W', 'A', 'V', 'E'};
    char     fmt[4] = {'f', 'm', 't', ' '};
    uint32_t fmtSize = 16;
    uint16_t format = 1;
    uint16_t channels = 2;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bits;
    char     data[4] = {'d', 'a', 't', 'a'};
    uint32_t dataSize;
};

constexpr size_t REC_BUFFER_SIZE = 512 * 1024; // 512KB für 2-3 Sekunden Puffer
constexpr size_t WRITE_CHUNK_SIZE = 1024;      // not too big!
constexpr size_t SD_FLUSH_INTERVAL = 65536;    // Alle 64KB flush
ps_ptr<uint8_t>  rec_buffer;
ps_ptr<uint8_t>  writeBuffer;

class AudioRecorder {
  public:
    std::atomic<size_t> writePos{0};
    std::atomic<size_t> readPos{0};
    uint32_t            totalBytes = 0;
    uint16_t            sampleRate = 44100;
    uint32_t            overflowCount = 0;

    volatile bool startRequested = false;
    volatile bool stopRequested = false;
    volatile bool running = false;

    bool push16(const int32_t* data, size_t frames) {
        // frames = Stereo-Frames
        size_t bytes16 = frames * 2 * sizeof(int16_t);

        size_t currentWrite = writePos.load(std::memory_order_relaxed);
        size_t currentRead = readPos.load(std::memory_order_acquire);

        size_t free = (currentRead + REC_BUFFER_SIZE - currentWrite - 1) % REC_BUFFER_SIZE;

        if (bytes16 > free) {
            overflowCount++;
            return false;
        }

        for (size_t i = 0; i < frames; i++) {
            // 32 → 16 Bit (High word)
            int32_t v = data[i] >> 16;

            // Optional Clipping (sicher)
            if (v > 32767) v = 32767;
            if (v < -32768) v = -32768;

            int16_t s = (int16_t)v;

            // Write byte by byte (LE)
            rec_buffer[currentWrite] = s & 0xFF;
            currentWrite = (currentWrite + 1) % REC_BUFFER_SIZE;
            rec_buffer[currentWrite] = (s >> 8) & 0xFF;
            currentWrite = (currentWrite + 1) % REC_BUFFER_SIZE;
        }

        writePos.store(currentWrite, std::memory_order_release);
        return true;
    }

    // Copies data to dest, returns bytes actually read
    size_t pop(uint8_t* dest, size_t maxLen) {
        size_t currentRead = readPos.load(std::memory_order_relaxed);
        size_t currentWrite = writePos.load(std::memory_order_acquire);

        if (currentRead == currentWrite) return 0;

        size_t avail = (currentWrite > currentRead) ? (currentWrite - currentRead) : (REC_BUFFER_SIZE - currentRead);

        size_t toRead = std::min(avail, maxLen);

        // Wrap-around handling
        size_t firstChunk = std::min(toRead, REC_BUFFER_SIZE - currentRead);
        memcpy(dest, &rec_buffer[currentRead], firstChunk);
        if (toRead > firstChunk) { memcpy(dest + firstChunk, &rec_buffer[0], toRead - firstChunk); }

        readPos.store((currentRead + toRead) % REC_BUFFER_SIZE, std::memory_order_release);
        return toRead;
    }

    // For external access to buffers (e.g. for pop with pointer math, but not recommended)
    size_t available() {
        size_t w = writePos.load(std::memory_order_acquire);
        size_t r = readPos.load(std::memory_order_acquire);
        return (w >= r) ? (w - r) : (REC_BUFFER_SIZE - r + w);
    }
};

AudioRecorder recorder;

void wavWriterTask(void*) {
    File      file;
    WAVHeader hdr;
    bool      fileOpen = false;

    size_t   writeBufferFill = 0;
    uint32_t bytesSinceFlush = 0;

    while (true) {
        // --- START REQUEST ---
        if (recorder.startRequested && !fileOpen) {
            recorder.startRequested = false;

            // Datei mit Zeitstempel erstellen
            char filename[64];
            snprintf(filename, sizeof(filename), "/audiofiles/recording.wav");

            file = SD_MMC.open(filename, FILE_WRITE);
            if (!file) {
                MWR_LOG_ERROR("Failed to open file! \"/audiofiles/recording.wav\"");
                continue;
            }

            // prepeare header
            hdr.sampleRate = recorder.sampleRate;
            hdr.byteRate = recorder.sampleRate * 2 * 2; // Stereo, 16-bit
            hdr.blockAlign = 2 * 2;                     // 8 bytes per frame
            hdr.bits = 16;
            hdr.dataSize = 0;
            hdr.size = 36; // 44 - 8 (RIFF header)

            file.write((uint8_t*)&hdr, sizeof(hdr));
            recorder.totalBytes = 0;
            writeBufferFill = 0;
            bytesSinceFlush = 0;
            fileOpen = true;
            recorder.running = true;
            rec_buffer.clear();
            writeBuffer.clear();
            printfln(s_tag.recorder, ANSI_ESC_YELLOW "Recording started: " ANSI_ESC_YELLOW "{}", filename);
        }

        // --- WRITE DATA ---
        if (fileOpen) {
            // fill local buffer
            while (writeBufferFill < WRITE_CHUNK_SIZE) {
                size_t spaceInLocalBuffer = WRITE_CHUNK_SIZE - writeBufferFill;
                size_t bytesRead = recorder.pop(writeBuffer + writeBufferFill, spaceInLocalBuffer);

                if (bytesRead == 0) break; // ringbuffer is empty

                writeBufferFill += bytesRead;
            }

            // write full block to SD
            if (writeBufferFill >= WRITE_CHUNK_SIZE) {
                size_t written = file.write(writeBuffer.get(), WRITE_CHUNK_SIZE);
                if (written != WRITE_CHUNK_SIZE) {
                    MWR_LOG_ERROR("SD write error!");
                    // Optional: Fehlerbehandlung, Buffer zurückhalten?
                }

                recorder.totalBytes += written;
                bytesSinceFlush += written;
                writeBufferFill = 0; // buffer is empty (or move remaining data)

                // Periodischer Flush für Datenintegrität
                if (bytesSinceFlush >= SD_FLUSH_INTERVAL) {
                    file.flush();
                    bytesSinceFlush = 0;
                }
            }
        }

        // --- STOP REQUEST ---
        if (recorder.stopRequested && fileOpen) {
            recorder.stopRequested = false;

            // Write remaining data to local buffer
            if (writeBufferFill > 0) {
                file.write(writeBuffer.get(), writeBufferFill);
                recorder.totalBytes += writeBufferFill;
            }

            // Update header
            hdr.dataSize = recorder.totalBytes;
            hdr.size = recorder.totalBytes + 36;

            file.seek(0);
            file.write((uint8_t*)&hdr, sizeof(hdr));
            file.flush();
            file.close();

            fileOpen = false;
            writeBufferFill = 0;
            recorder.running = false;
            printfln(s_tag.recorder, "Recording stopped. Total bytes: " ANSI_ESC_CYAN "{}" ANSI_ESC_RESET ", Overflows: " ANSI_ESC_CYAN "{}", recorder.totalBytes, recorder.overflowCount);
        }

        // Small delay to feed watchdog and release CPU
        // But not too long, so that the ring buffer does not overflow!
        vTaskDelay(pdMS_TO_TICKS(1)); // 1ms = ~176 Bytes bei 44.1kHz Stereo 32-bit
    }
}

void audio_process_raw_samples(int32_t* outBuff, int16_t validSamples) {
    if (recorder.running == true) { recorder.push16(outBuff, validSamples); }
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char ir_symbols[34][15] = {"ZERO",        "ONE",        "TWO",        "THREE",     "FOUR",    "FIFE",    "SIX",  "SEVEN",        "EIGHT",    "NINE",    "MUTE",
                                 "ARROW_RIGHT", "ARROW_LEFT", "ARROW_DOWN", "ARROW_UP",  "MODE",    "OKAY",    "N/A",  "PAUSE/RESUME", "STOP",     "ON/OFF",  "RADIO",
                                 "PLAYER",      "DLNA",       "CLOCK",      "OFF_TIMER", "VOLUME+", "VOLUME-", "-30s", "+30s",         "CHANNEL+", "CHANNEL-"};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  I R _ B U T T O N S  📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class IR_buttons {
  private:
    settings_s* m_settings;
    uint8_t     m_numOfIrButtons = 0;

  public:
    IR_buttons(settings_s* s) {
        m_settings = s;
        m_numOfIrButtons = 0;
    }
    ~IR_buttons() {}

    const char* skipWhitespace(const char* str) { // Helper function: Skip spaces
        while (*str && isspace(*str)) { str++; }
        return str;
    }

    const char* extractString(const char* ptr, ps_ptr<char>& dest) {
        if (*ptr != '"') {
            Serial.println("Error: Expected string.");
            return nullptr;
        }
        ++ptr;
        const char* start = ptr;
        while (*ptr && *ptr != '"') { ++ptr; }
        if (*ptr != '"') {
            Serial.println("Error: Unterminated string.");
            return nullptr;
        }
        dest.assign(start, ptr - start);
        return ptr + 1;
    }

    bool parseJSONString(ps_ptr<char> jsonString) { // Function to parse the JSON string
        const char*  ptr = jsonString.get();
        uint8_t      buttonNr = 0;
        size_t       buttonIndex = 0;
        ps_ptr<char> v;

        // Check if the JSON string starts with '['
        ptr = skipWhitespace(ptr);
        if (*ptr != '[') {
            Serial.println("Error: Expected '[' to start array.");
            return false;
        }
        ptr++; // Überspringe '['

        // Process each JSON object in the array
        while (*ptr && *ptr != ']' && buttonIndex < 43) {
            ptr = skipWhitespace(ptr);

            if (*ptr != '{') {
                Serial.println("Error: Expected '{' to start an object.");
                return false;
            }

            ptr++; // skip '{'

            int16_t      val = -1;
            ps_ptr<char> label;
            bool         validObject = false;

            while (*ptr && *ptr != '}') {
                ps_ptr<char> value;
                ps_ptr<char> key;

                ptr = skipWhitespace(ptr);

                // Schlüssel extrahieren
                if (*ptr == '\"') {
                    ptr++; // skip '"'
                    const char* keyStart = ptr;
                    while (*ptr && *ptr != '\"') { ptr++; }
                    if (*ptr != '"') {
                        Serial.println("Error: Unterminated key.");
                        return false;
                    }
                    key.assign(keyStart, ptr - keyStart);
                    ptr++; // skip '"'
                    ptr = skipWhitespace(ptr);

                    if (*ptr == ':') {
                        ptr++; // skip ':'
                        ptr = skipWhitespace(ptr);
                        // Value based on key
                        if (key[0] == 'A') { // IR Address
                            buttonNr = 42;
                            ptr = extractString(ptr, value);
                            if (!ptr) return false; // error found
                            val = value.to_int16(); // Hex in uint8_t umwandeln
                            validObject = true;
                        } else if (key[0] == 'C') {
                            ; // IR command unused
                            buttonNr = 43;
                            ptr = extractString(ptr, value);
                            if (!ptr) return false; // error found
                            val = value.to_int16(); // Hex in uint8_t umwandeln
                            validObject = true;
                        } else if (isdigit(key[0])) { // Nummer, z.B. "0", "10"
                            buttonNr = key.to_uint8();
                            ptr = extractString(ptr, value);
                            if (!ptr) return false; // error found
                            val = value.to_int16(); // Hex in uint8_t umwandeln
                            validObject = true;
                        } else if (key == "label") { // Label
                            ptr = extractString(ptr, label);
                            if (!ptr) return false; // error found
                        }
                    }
                }

                ptr = skipWhitespace(ptr);
                if (*ptr == ',') {
                    ptr++; // skip ','
                }
            }

            // Make sure both values are present
            if (validObject && label.valid()) {
                m_settings->irbuttons[buttonNr].val = val;
                m_settings->irbuttons[buttonNr].label = label;
                // MWR_LOG_WARN("buttonNr {}, val {}, label {}", buttonNr, m_settings->irbuttons[buttonNr].val, m_settings->irbuttons[buttonNr].label);
                buttonIndex++;
            } else {
                Serial.println("Error: Invalid object, missing buttonNr or label.");
                return false;
            }

            ptr = skipWhitespace(ptr);
            if (*ptr == '}') {
                ptr++; // skip '}'
            }

            ptr = skipWhitespace(ptr);
            if (*ptr == ',') {
                ptr++; // skip ','
            }
        }

        // Check that the array ends correctly with ']'
        ptr = skipWhitespace(ptr);
        if (*ptr != ']') {
            Serial.println("Error: Expected ']' to close array.");
            return false;
        }
        return true; // JSON parsed successfully
    }

    uint8_t loadButtonsFromJSON(const char* filename) {
        File file = SD_MMC.open(filename);
        if (!file) {
            Serial.println("Failed to open file");
            return 0;
        }
        ps_ptr<char> jsonString;
        while (file.available()) {
            char c = file.read();
            jsonString.append(&c, 1);
        }
        file.close();
        // JSON parsen
        if (!parseJSONString(jsonString)) {
            Serial.println("Failed to parse JSON.");
            return 0;
        }
        // Anzahl der IR-Buttons ermitteln
        m_numOfIrButtons = 0;
        while (m_numOfIrButtons < 45 && !m_settings->irbuttons[m_numOfIrButtons].label.empty()) { ++m_numOfIrButtons; }
        m_settings->numOfIrButtons = m_numOfIrButtons;
        return m_numOfIrButtons;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
