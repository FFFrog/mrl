#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Solution {
public:
    Solution()
    {
    }
    int maxScoreSightseeingPair(vector<int> &values)
    {
        int n = values.size();
        int max = values[0];
        int rec = 0;

        for (int j = 1; j < n; j++) {
            max = (values[j - 1] + j - 1) > max ? values[j - 1] + j - 1 : max;
            rec = max + values[j] - j > rec ? max + values[j] - j : rec;
        }

        return rec;
    }
};

int main()
{
    vector<int> vec = {8, 1, 5, 2, 6};

    Solution sol;
    cout << sol.maxScoreSightseeingPair(vec) << endl;

    return 0;
}
