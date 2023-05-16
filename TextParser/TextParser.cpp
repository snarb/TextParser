#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <locale>
#include <codecvt>
#define NOMINMAX
#include <Windows.h>

#include <locale>
#include <codecvt>
#include <cwctype>
#include <algorithm>
#include <cctype>
#include <iostream>


#include <set>
#include <fstream>
#include <sstream>
#include <unordered_map>

constexpr int CHUNK_SIZE_MIN = 10;
constexpr int MIN_WORD_LENGTH = 2;
struct Words {
    std::wstring cleaned_text;
    std::size_t start;
    std::size_t end;
};

std::set<std::wstring> allowed_words;
std::unordered_map<std::wstring, int> counter;
std::vector<Words> words;



std::vector<std::wstring> split_into_chunks(const std::vector<std::wstring>&words) {
    std::vector<std::wstring> chunks;
    std::wstring current_chunk;
    for (const auto& word : words) {
        if (allowed_words.count(word) == 0) {
            counter[word]++;

            if (current_chunk.size() >= CHUNK_SIZE_MIN) {
                chunks.push_back(current_chunk);
            }
            current_chunk.clear();
        }
        current_chunk += (word + L" ");
    }

    if (current_chunk.size() >= CHUNK_SIZE_MIN) {
        chunks.push_back(current_chunk);
    }
    return chunks;
}

void read_words_into_set(std::set<std::wstring>&words, const std::string & filename) {
    std::ifstream file(filename);
    if (file) {
        std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);

        std::wstringstream wss(wstr);
        std::wstring word;
        while (std::getline(wss, word, L'\n')) {
            words.insert(word);
        }
    }
    else {
        std::wcerr << L"Could not open file " << filename.c_str() << std::endl;
    }
}



std::wstring clean_word(std::wstring word) {
    // Remove non-alphanumeric characters
    word.erase(std::remove_if(word.begin(), word.end(), [](wchar_t c) { return !std::iswalnum(c); }), word.end());

    // Check if the word contains a digit or is less than MIN_WORD_LENGTH, if so return an empty string
    if (std::any_of(word.begin(), word.end(), [](wchar_t c) { return std::iswdigit(c); }) || word.length() < MIN_WORD_LENGTH) {
        return L"";
    }

    // Convert to lowercase
    std::transform(word.begin(), word.end(), word.begin(), [](wchar_t c) { return std::towlower(c); });

    return word;
}

// A helper function to split a string into words words.clear();
void split_string(const std::wstring& str) {
    std::wcout << L"Before printing number of chunks." << std::endl;
    words.clear();
    std::wstring word;
    std::size_t start = 0;

    for (std::size_t i = 0; i < str.size(); ++i) {
        wchar_t c = str[i];
        if (c == L' ' || c == L'\n' || c == L'\r') {
            if (!word.empty()) {
                words.push_back({ clean_word(word), start, i - 1 });
                word.clear();
            }
            // update start position after the separator
            start = i + 1;
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back({ clean_word(word), start, str.size() - 1 });
    }

    words.erase(std::remove_if(words.begin(), words.end(), [](const Words& chunk) {
        return chunk.cleaned_text.empty();
        }), words.end());

    std::wcout << L"Number of chunks: " << words.size() << std::endl;
}


std::wstring readFile(const std::string & filename) {
    std::ifstream file(filename, std::ios::binary);
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);

    return wstr;
}

void read_tx_files(const std::filesystem::path & path) {
    int iter = 0;
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                try
                {
                    std::wstring content = readFile(entry.path().string());

                    split_string(content);

                   // auto chunks = split_into_chunks(words);
                   // std::wcout << L"Number of chunks: " << chunks.size() << std::endl;

                    //for (const auto& cleaned_text : chunks) {
                    //    std::wcout << L"Chunk: " << cleaned_text.cleaned_text << std::endl;
                    //    std::wcout << L"Start position: " << cleaned_text.start << std::endl;
                    //    std::wcout << L"End position: " << cleaned_text.end << std::endl;
                    //    std::wcout << L"Source: ";
                    //    for (std::size_t i = cleaned_text.start; i <= cleaned_text.end; ++i) {
                    //        std::wcout << content[i];
                    //    }
                    //    std::wcout << std::endl;
                    //}

                    // Print the most frequent words
                    if (iter % 100 == 0) {
                        //--------
                        // Sort the words by frequency
                        std::vector<std::pair<std::wstring, int>> wordFrequencies;
                        wordFrequencies.reserve(counter.size());  // Reserve space for efficient memory allocation

                        // Copy key-value pairs from the counter map to the wordFrequencies vector
                        for (const auto& pair : counter) {
                            wordFrequencies.emplace_back(pair.first, pair.second);
                        }

                        // Sort the word frequencies in descending order
                        std::sort(wordFrequencies.begin(), wordFrequencies.end(),
                            [](const std::pair<std::wstring, int>& a, const std::pair<std::wstring, int>& b) {
                                return a.second > b.second; // Sort in descending order
                            });

                        //std::system("cls"); // Clear the screen (Windows)
                        std::locale::global(std::locale(""));
                        std::wofstream outputFile("C:\\repos\\TextParser\\output.txt", std::ios::out | std::ios::app);
                        outputFile.imbue(std::locale());

                        int numWordsToPrint = 300;
                        for (int i = 0; i < numWordsToPrint && i < wordFrequencies.size(); ++i) {
                            const auto& wordFrequency = wordFrequencies[i];
                            outputFile << wordFrequency.first << std::endl;
                            outputFile.flush();
                            //std::wcout << wordFrequency.first << std::endl; //<< L", Frequency: " << wordFrequency.second << std::endl;
                        }

                        outputFile.close(); // Close the file stream

                        int g = 2;
                    }
                    iter++;
                    //-------------
                }
                catch (const std::exception& ex)
                {
                    // Handle the exception, display an error message, or perform any necessary actions
                    std::cerr << "An exception occurred: " << ex.what() << std::endl;
                }

            }
        }
    }
}


int main(int argc, char* argv[]) {
    //if (argc < 2) {
    //    std::cerr << "Please provide a directory." << std::endl;
    //    return 1;
    //}
    //std::locale::global(std::locale("")); // Set the global locale to the user's default locale
    //std::wcout.imbue(std::locale()); // Set the locale for std::wcout
    std::string data_dir = "C:\\repos\\TextParser\\";
    read_words_into_set(allowed_words, data_dir + "lemmas_all.txt");
    read_words_into_set(allowed_words, data_dir + "cities.txt");
    read_words_into_set(allowed_words, data_dir + "countries.txt");
    read_words_into_set(allowed_words, data_dir + "extra.txt");
    read_words_into_set(allowed_words, data_dir + "f_names.txt");
    read_words_into_set(allowed_words, data_dir + "s_names.txt");


    //std::filesystem::path path("C:\\datasets\\books_txt\\lit_Children"); //argv[1]
    std::filesystem::path path("C:\\repos\\TextParser\\TextParser\\test"); //argv[1]
    read_tx_files(path);
    return 0;
}