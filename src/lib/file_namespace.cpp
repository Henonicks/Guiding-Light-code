#include <file_namespace.h>

void file::line_append(const std::string& s, std::string filename) {
    std::string line;
    std::ifstream ifile;
    ifile.open(filename);
    std::ofstream otemp;
    otemp.open(temp);
    otemp.clear();
    while (std::getline(ifile, line)) {
        otemp << line << std::endl;
    }
    ifile.close();
    otemp.close();
    std::ofstream ofile;
    ofile.open(filename);
    std::ifstream itemp;
    itemp.open(temp);
    while (std::getline(itemp, line)) {
        ofile << line << std::endl;
    }
    ofile << s << std::endl;
    itemp.close();
    ofile.close();
}

std::string file::getline(const std::string& s, const std::string& filename, int start_position) {
    std::string line;
    std::ifstream ifile;
    ifile.open(filename);
    while (std::getline(ifile, line)) {
        if (line.find(s) == start_position || (start_position == -1 && line.find(s) != std::string::npos)) {
            return line;
        }
    }
    return "";
}

void file::delete_line_once(const std::string& s, const std::string& filename, const bool& should_contain_at_pos_0) {
    std::ifstream ifile;
    ifile.open(filename);
    std::ofstream otemp;
    otemp.open(temp);
    otemp.clear();
    std::string line;
    while (std::getline(ifile, line)) {
        if (should_contain_at_pos_0) {
            if (line.find(s) != 0) {
                otemp << line << std::endl;
            }
        }
        else if (line != s) {
            otemp << line << std::endl;
        }
    }
    ifile.close();
    otemp.close();
    std::ofstream ofile;
    ofile.open(filename);
    std::ifstream itemp;
    itemp.open(temp);
    ofile.clear();
    while (std::getline(itemp, line)) {
        ofile << line << std::endl;
    }
    itemp.close();
    otemp.close();
}

void file::delete_if_line(const std::string& s, std::string filename) {
    std::string line;
    std::ifstream ifile;
    ifile.open(filename);
    std::ofstream otemp;
    while (std::getline(ifile, line)) {
        if (line != s) {
            otemp << line << std::endl;
        }
    }
    std::ofstream ofile;
    ofile.open(filename);
    std::ifstream itemp;
    itemp.open(filename);
    ofile.clear();
    while (std::getline(itemp, line)) {
        ofile << line << std::endl;
    }
    itemp.close();
    ifile.close();
    otemp.close();
    ofile.close();
}

void file::line_attach(const std::string& s, const std::string& filename, int pos) {
    std::string line;
    std::ifstream ifile;
    std::ofstream otemp;
    otemp.clear();
    ifile.open(filename);
    otemp.open(temp);
    int currpos = 0;
    std::string line_in_pos;
    while (std::getline(ifile, line)) {
        if (currpos != pos) {
            otemp << line << std::endl;
        }
        else {
            line_in_pos = line;
        }
        currpos++;
    }
    ifile.close();
    otemp.close();
    std::ofstream ofile;
    std::ifstream itemp;
    ofile.open(filename);
    itemp.open(temp);
    ofile.clear();
    ofile << line_in_pos + s << std::endl;
    while (std::getline(itemp, line)) {
        ofile << line << std::endl;
    }
    ofile.close();
    itemp.close();
}

void file::delete_line_once(int pos, std::string filename) {
    std::string line;
    std::ifstream ifile;
    ifile.open(filename);
    int position = 0;
    while (std::getline(ifile, line)) {
        if (position == pos) {
            delete_line_once(line, filename);
            break;
        }
        position++;
    }
}

bool file::is_in_file(const std::string& s, std::string filename) {
    std::string line;
    std::ifstream file;
    file.open(filename);
    while (std::getline(file, line)) {
        if (line.find(s) == 0) {
            return true;
        }
    }
    return false;
}

int file::pos_in_file(const std::string& s, const std::string& filename) {
    std::string line;
    std::ifstream file;
    file.open(filename);
    int pos = 0;
    while (std::getline(file, line)) {
        if (line.find(s) == 0) {
            return pos;
        }
        pos++;
    }
    return -1;
}