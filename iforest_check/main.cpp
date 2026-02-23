#include <iostream>
#include "iforest_model.h"

using namespace std;

// =============================
// SCALE INPUT
// =============================
void scale_input(float *x) {
    for (int i = 0; i < NUM_FEATURES; i++) {
        x[i] = (x[i] - scaler_mean[i]) / scaler_scale[i];
    }
}

// =============================
// TREE TRAVERSAL
// =============================
float path_length(
    const int *feature,
    const float *threshold,
    const int *left,
    const int *right,
    float *x)
{
    int node = 0;
    float depth = 0.0f;

    while (left[node] != -1) {
        int f = feature[node];

        if (x[f] <= threshold[node]) {
            node = left[node];
        } else {
            node = right[node];
        }

        depth += 1.0f;
    }

    return depth;
}

// =============================
// COMPUTE SCORE
// =============================
float compute_score(float *x) {
    float total = 0.0f;

    for (int i = 0; i < NUM_TREES; i++) {
        total += path_length(
            tree_feature[i],
            tree_threshold[i],
            tree_left[i],
            tree_right[i],
            x
        );
    }

    float avg = total / NUM_TREES;

    // match your Python logic
    return -avg;
}

// =============================
// MAIN TEST
// =============================
int main() {

    // ==========================================
    // 🔴 PUT YOUR DATA HERE (IMPORTANT PART)
    // ==========================================
    float sample[NUM_FEATURES] = {
        1000123, 1000123, 1000123, 92.1000123, 210001230, 100012320, 210001230, 100012314, 
    0.022528, 231000123, 1000123, 100012320, 100012320, 100012320, 100012320, 100012320, 210001230
    };

    // Step 1: scale
    scale_input(sample);

    // Step 2: score
    float score = compute_score(sample);

    // Step 3: classify
    int is_anomaly = (score > IF_THRESHOLD) ? 1 : 0;

    // Output
    cout << "Score: " << score << endl;
    cout << "Threshold: " << IF_THRESHOLD << endl;
    cout << "Anomaly: " << is_anomaly << endl;

    return 0;
}