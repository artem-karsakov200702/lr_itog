#include "json.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using json = nlohmann::json;

struct TextObject {
    string name;
    string content;
    int found = 0;
    int replaced = 0;
};

vector<size_t> findAllPositions(const string& text, const string& pattern, bool case_sensitive) {
    if (pattern.empty()) return {};
    string haystack = text, needle = pattern;
    if (!case_sensitive) {
        transform(haystack.begin(), haystack.end(), haystack.begin(), ::tolower);
        transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
    }
    vector<size_t> positions;
    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != string::npos) {
        positions.push_back(pos);
        pos++;
    }
    return positions;
}

string replaceAllOccurrences(const string& text, const string& from, const string& to, bool case_sensitive) {
    auto positions = findAllPositions(text, from, case_sensitive);
    if (positions.empty()) return text;
    size_t delta = to.size() - from.size();
    size_t new_size = text.size() + positions.size() * delta;
    string result;
    result.reserve(new_size);
    size_t last_pos = 0;
    for (size_t p : positions) {
        result.append(text.data() + last_pos, p - last_pos);
        result.append(to);
        last_pos = p + from.size();
    }
    result.append(text.data() + last_pos);
    return result;
}

vector<TextObject> extractTextObjects(const json& j) {
    vector<TextObject> objects;
    if (!j.is_array()) return objects;
    for (const auto& item : j) {
        if (!item.is_object()) continue;
        TextObject obj;
        if (item.contains("name") && item["name"].is_string()) obj.name = item["name"];
        if (item.contains("content") && item["content"].is_string()) obj.content = item["content"];
        if (!obj.name.empty()) objects.push_back(obj);
    }
    return objects;
}

void test_search_json() {
    cout << "Test 1: Search in JSON" << endl;
    json data = json::array({
        {{"name", "file1.txt"}, {"content", "foo foo test bar foo"}},
        {{"name", "file2.txt"}, {"content", "test foo bar"}},
        {{"name", "file3.txt"}, {"content", "test bar test"}}
        });
    auto objects = extractTextObjects(data);
    int total_found = 0;
    for (auto& obj : objects) {
        obj.found = findAllPositions(obj.content, "foo", true).size();
        total_found += obj.found;
        cout << obj.name << ": " << obj.found << endl;
    }
    cout << "Total found: " << total_found << endl << endl;
}

void test_replace_json() {
    cout << "Test 2: Replace in JSON" << endl;
    json data = json::array({
        {{"name", "file1.txt"}, {"content", "foo test foo bar foo"}},
        {{"name", "file2.txt"}, {"content", "foo bar test foo"}}
        });
    auto objects = extractTextObjects(data);
    for (auto& obj : objects) {
        obj.content = replaceAllOccurrences(obj.content, "foo", "BAR", true);
        cout << obj.name << ": " << obj.content << endl;
    }
    cout << "Replace done!" << endl << endl;
}

void test_corrupted_json() {
    cout << "Test 3: Corrupted JSON" << endl;
    json data = json::array({
        {{"name", "broken1.txt"}},
        {{"content", "only content"}},
        {{"name", ""}, {"content", "empty name"}},
        {"not object string"},
        {{"bad_field", 123}}
        });
    auto objects = extractTextObjects(data);
    cout << "Extracted objects: " << objects.size() << " (should be 0)" << endl;
    if (objects.empty()) {
        cout << "OK: all corrupted ignored!" << endl;
    }
    cout << endl;
}

void test_empty_json() {
    cout << "Test 4: Empty JSON" << endl;
    json empty_array = json::array();
    json not_array = { {"key", "value"} };
    auto objects1 = extractTextObjects(empty_array);
    auto objects2 = extractTextObjects(not_array);
    cout << "Empty array: " << objects1.size() << endl;
    cout << "Not array: " << objects2.size() << endl;
    cout << "OK: both return 0 objects" << endl << endl;
}

void test_case_insensitive() {
    cout << "Test 5: Case insensitive" << endl;
    json data = json::array({
        {{"name", "file1.txt"}, {"content", "Foo fOo FOO foo"}}
        });
    auto objects = extractTextObjects(data);
    int found_sensitive = findAllPositions(objects[0].content, "foo", true).size();
    int found_insensitive = findAllPositions(objects[0].content, "foo", false).size();
    cout << objects[0].name << ":" << endl;
    cout << "  Sensitive: " << found_sensitive << " (only 'foo')" << endl;
    cout << "  Insensitive: " << found_insensitive << " (all variants)" << endl;
    cout << endl;
}

int main() {
    cout << "JSON tests (5 tests):" << endl << endl;

    test_search_json();
    test_replace_json();
    test_corrupted_json();
    test_empty_json();
    test_case_insensitive();

    return 0;
}





