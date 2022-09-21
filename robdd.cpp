#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

int pow(int base, int exp);
void printTable(vector<int> table);
void printRow(vector<int> row);
void processOutput(vector<int> table, string filename);
bool hasSameChildren(vector<int> row);
bool andRows(vector<int> row1, vector<int> row2);
bool rowExists(vector<int> table, vector<int> row);
vector<int> fillOnes(vector<int> table, string expression);
vector<int> generateTable(int var_num, vector<char> var_names);
vector<int> deleteRepeatedRows(vector<int> table, int var);
vector<int> getRow(vector<int> table, int line);
vector<int> deleteAllRepeatedRows(vector<int> table, vector<char> var_names);
vector<int> deleteSameChildren(vector<int> table);
vector<int> processInput(string filename);
vector<int> reduceTable(vector<int> table);

int main(int argc, char *argv[])
{
    vector<int> table = processInput(argv[1]);
    table = reduceTable(table);
    printTable(table);
    processOutput(table, argv[2]);
}

void processOutput(vector<int> table, string filename)
{
    const int table_length = 4;
    const int true_branch = 3;
    const int false_branch = 2;
    const int var_branch = 1;
    const int num_rows = table.size() / table_length;

    string content;

    content += "digraph ROBDD {\n";

    int last_var = -1;
    string tt = "\t{rank=same ";
    for (int i = 1; i < num_rows - 1; i++)
    {
        int index = table[i * table_length];
        int var = table[i * table_length + var_branch];

        if (var == last_var)
        {
            tt += " ";
            tt += to_string(index);
        }
        else
        {
            last_var = var;
            tt += "}\n";

            if (tt.size() > 14)
                content += tt;

            tt = "\t{rank=same ";
            tt += to_string(index);
        }
    }

    tt += "}\n";
    content += tt;
    content += '\n';

    for (int i = 0; i < num_rows; i++)
    {
        int index = table[i * table_length];
        string temp = "\t";

        if (i == 0 || i == num_rows - 1)
        {
            temp += to_string(index);
            temp += "[label=\"";
            temp += i == 0 ? '0' : '1';
            temp += "\",shape=box]\n";
        }
        else
        {
            temp += to_string(index);
            temp += "[label=\"";
            temp += char(table[i * table_length + var_branch]);
            temp += "\"]\n";
        }

        content += temp;
    }
    content += '\n';

    for (int i = 1; i < num_rows - 1; i++)
    {
        string t = "\t";
        t += to_string(table[i * table_length]);
        t += "->";
        t += to_string(table[i * table_length + false_branch]);
        t += "[label=\"0\",style=dotted]\n";
        content += t;

        t = "\t";
        t += to_string(table[i * table_length]);
        t += "->";
        t += to_string(table[i * table_length + true_branch]);
        t += "[label=\"1\",style=solid]\n";
        content += t;
    }
    content += '}';

    ofstream myFile(filename);
    myFile << content;
    myFile.close();
}

vector<int> processInput(string filename)
{
    string data;
    ifstream fin;
    int lineNum = 1;

    int var_nums;
    vector<char> var_names;
    vector<string> code_list;

    fin.open(filename);
    while (fin)
    {
        getline(fin, data);

        if (lineNum == 1)
        {
            stringstream ss(data);
            string a;

            ss >> a;
            ss >> var_nums;
        }
        else if (lineNum == 2)
        {
        }
        else if (lineNum == 3)
        // Read variable names
        {
            stringstream ss(data);
            string a;
            char b;
            ss >> a;

            for (int i = 0; i < var_nums; i++)
            {
                ss >> b;
                var_names.push_back(b);
            }
        }
        else if (lineNum > 5)
        {
            const string endline = ".e";

            if (data == endline)
                break;

            stringstream ss(data);
            string a;
            ss >> a;

            code_list.push_back(a);
        }

        lineNum++;
    }

    fin.close();

    vector<int> table = generateTable(var_nums, var_names);

    for (int i = 0; i < code_list.size(); i++)
        table = fillOnes(table, code_list[i]);

    table = deleteAllRepeatedRows(table, var_names);

    for (int i = 0; i < 5; i++)
        table = deleteSameChildren(table);

    return table;
}

vector<int> reduceTable(vector<int> table)
{
    vector<int> temp;
    const int table_length = 4;
    const int true_branch = 3;
    const int false_branch = 2;
    const int num_rows = table.size() / table_length;

    for (int i = 0; i < num_rows; i++)
    {
        vector<int> temp_row;
        temp_row.push_back(table[i * table_length]);
        temp_row.push_back(table[i * table_length + 1]);
        temp_row.push_back(table[i * table_length + 2]);
        temp_row.push_back(table[i * table_length + 3]);

        if ((!rowExists(temp, temp_row) && !hasSameChildren(temp_row)) || i == 0 || i == num_rows - 1)
            for (int i = 0; i < temp_row.size(); i++)
                temp.push_back(temp_row[i]);
    }

    return temp;
}

bool rowExists(vector<int> table, vector<int> row)
{
    const int table_length = 4;
    const int num_rows = table.size() / table_length;

    for (int i = 0; i < num_rows; i++)
        if (table[i * table_length + 1] == row[1] && table[i * table_length + 2] == row[2] && table[i * table_length + 3] == row[3])
            return true;

    return false;
}

vector<int> deleteSameChildren(vector<int> table)
{
    const int table_length = 4;
    const int true_branch = 3;
    const int false_branch = 2;
    const int num_rows = table.size() / table_length - 1;

    for (int i = num_rows - 1; i > 0; i--)
    {
        vector<int> r = getRow(table, i);
        if (hasSameChildren(r))
        {
            for (int j = 1; j < i; j++)
            {
                if (table[j * table_length + true_branch] == i)
                    table[j * table_length + true_branch] = table[i * table_length + true_branch];
                else if (table[j * table_length + false_branch] == i)
                    table[j * table_length + false_branch] = table[i * table_length + false_branch];
            }
        }
    }

    return table;
}

bool hasSameChildren(vector<int> row)
{
    if (row[2] == row[3])
        return true;
    return false;
}

vector<int> deleteAllRepeatedRows(vector<int> table, vector<char> var_names)
{
    for (int i = var_names.size() - 1; i >= 0; i--)
        table = deleteRepeatedRows(table, int(var_names[i]));

    return table;
}

vector<int> deleteRepeatedRows(vector<int> table, int var)
{
    const int table_length = 4;
    const int true_branch = 3;
    const int false_branch = 2;
    const int num_rows = table.size() / table_length;

    for (int i = 1; i < num_rows - 1; i++)
    {
        int v = table[table_length * i + 1];

        if (v == var)
        {
            for (int j = i + 1; j < num_rows - 1; j++)
            {
                if (table[table_length * j + 1] != var)
                    break;

                vector<int> row1 = getRow(table, i);
                vector<int> row2 = getRow(table, j);

                if (andRows(row1, row2))
                {
                    for (int k = 1; k < num_rows - 1; k++)
                    {
                        if (table[k * table_length + true_branch] == j)
                            table[k * table_length + true_branch] = i;
                        if (table[k * table_length + false_branch] == j)
                            table[k * table_length + false_branch] = i;
                    }
                }
            }
        }
    }

    return table;
}

vector<int> getRow(vector<int> table, int line)
{
    const int table_length = 4;

    vector<int> temp;
    int i = line * table_length;

    for (int k = 0; k < table_length; k++)
        temp.push_back(table[i + k]);

    return temp;
}

bool andRows(vector<int> row1, vector<int> row2)
{
    if (row1[1] == row2[1] && row1[2] == row2[2] && row1[3] == row2[3])
        return true;
    return false;
}

vector<int> fillOnes(vector<int> table, string expression)
{
    const int true_branch = 3;
    const int false_branch = 2;
    const int table_length = 4;
    const char true_char = '1';
    const char false_char = '0';
    const char dash_char = '-';

    int last_index = table.size() / table_length - 2;
    const int last_var = table[last_index * table_length + 1];

    while (table[last_index * table_length + 1] == last_var)
    {
        string expr = "";
        int temp = last_index - 1;
        int memo = last_index;

        while (temp >= 1)
        {
            if (table[temp * table_length + true_branch] == memo || table[temp * table_length + false_branch] == memo)
            {
                if (memo % 2 == 0)
                    expr = false_char + expr;
                else
                    expr = true_char + expr;

                memo = temp;
            }

            temp--;
        }

        string temp_expr_1 = expr + true_char;
        string temp_expr_2 = expr + false_char;

        bool fillTrue = true;
        for (int i = 0; i < expression.size(); i++)
        {
            if (expression[i] == true_char && temp_expr_1[i] != true_char)
                fillTrue = false;
            else if (expression[i] == false_char && temp_expr_1[i] != false_char)
                fillTrue = false;
        }
        if (fillTrue)
            table[last_index * table_length + true_branch] = table.size() / table_length - 1;

        fillTrue = true;
        for (int i = 0; i < expression.size(); i++)
        {
            if (expression[i] == true_char && temp_expr_2[i] != true_char)
                fillTrue = false;
            else if (expression[i] == false_char && temp_expr_2[i] != false_char)
                fillTrue = false;
        }
        if (fillTrue)
            table[last_index * table_length + false_branch] = table.size() / table_length - 1;

        last_index--;
    }

    return table;
}

void printTable(vector<int> table)
{
    for (int i = 0; i < table.size(); i++)
    {
        if (i % 4 == 0 && i != 0)
            cout << endl;
        cout << table[i] << " ";
    }
    cout << endl;
}

vector<int> generateTable(int var_num, vector<char> var_names)
{
    vector<int> table;
    int var_count = 1;
    int max_index = pow(2, var_num);

    for (int index = 0; index <= max_index; index++)
    {
        if (index == 0)
        {
            table.push_back(index);
            table.push_back(0);
            table.push_back(0);
            table.push_back(0);
        }
        else if (index == max_index)
        {
            table.push_back(index);
            table.push_back(1);
            table.push_back(0);
            table.push_back(0);
        }
        else
        {
            table.push_back(index);

            if (index / pow(2, var_count) == 0)
                table.push_back(int(var_names[var_count - 1]));
            else
            {
                var_count++;
                table.push_back(int(var_names[var_count - 1]));
            }

            if (var_count != var_names.size())
            {
                table.push_back(2 * index);
                table.push_back(2 * index + 1);
            }
            else
            {
                table.push_back(0);
                table.push_back(0);
            }
        }
    }

    return table;
}

int pow(int base, int exp)
{
    int num = base;
    for (int i = 1; i < exp; i++)
        num *= base;

    return num;
}

void printRow(vector<int> row)
{
    cout << "Row: ";
    for (int i = 0; i < row.size(); i++)
        cout << row[i] << " ";
    cout << endl;
}