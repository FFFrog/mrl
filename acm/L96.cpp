#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    Solution(int n)
    {
        record.resize(n + 1);
        record[0] = 1;
    }

    int numTrees(int n)
    {
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= i; j++) {
                record[i] += record[j - 1] * record[i - j];
            }
        }

        return record[n];
    }

    int numTrees2(int n)
    {
        int ans = 0;

        if (record[n]) {
            return record[n];
        } else {
            for (int i = 1; i <= n; i++) {
                ans += numTrees2(i - 1) * numTrees2(n - i);
            }
            record[n] = ans;
        }

        return ans;
    }

private:
    vector<int> record;
};

int main()
{
    int n;

    cin >> n;

    Solution sol(n);
    cout << sol.numTrees(n) << endl;
    cout << sol.numTrees2(n) << endl;

    return 0;
}
