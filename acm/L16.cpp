#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Solution {
public:
    int threeSumClosest(vector<int> &nums, int target)
    {
        int n = nums.size();
        int diff = INT_MAX;
        int value = 0;
        int ans = 0;

        sort(nums.begin(), nums.end());

        for (int i = 0; i < n - 2; i++) {
            int j = i + 1;
            int z = n - 1;
            while (j < z) {
                value = nums[i] + nums[j] + nums[z];
                if (abs(value - target) < diff) {
                    diff = abs(value - target);
                    ans = value;
                } else if (value == target) {
                    return target;
                }
                value > target ? z-- : j++;
            }
        }

        return ans;
    }
};

int main()
{
    vector<int> vec = {0, 2, 1, -3};

    Solution sol;
    cout << sol.threeSumClosest(vec, 1) << endl;

    return 0;
}
