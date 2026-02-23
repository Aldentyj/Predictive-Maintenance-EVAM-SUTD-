#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstdint>

using namespace std;


struct CAN_Message {
    uint32_t identifier;
    uint8_t data[8] = {0};
};


struct Telemetry {

    float Vehicle_Speed_kmh = 0;
    float Wheel_RPM_FL = 0;
    float Wheel_RPM_FR = 0;
    float Wheel_RPM_RL = 0;
    float Wheel_RPM_RR = 0;

    float Battery_Voltage_V = 0;
    float Battery_Current_A = 0;
    float Battery_SOC_Pct = 0;
    float Battery_Resistance_Ohms = 0;
    float Battery_Temp_Max_C = 0;

    float Throttle_Pct = 0;
    float Brake_Pedal_Pct = 0;
    float Brake_Pressure_Front_kPa = 0;
    float Brake_Pressure_Rear_kPa = 0;

    float Target_Thr_FL = 0;
    float Target_Thr_FR = 0;
    float Target_Thr_RL = 0;
    float Target_Thr_RR = 0;

} T;



uint16_t getLE(uint8_t *data, int start) {
    return data[start] | (data[start + 1] << 8);
}

vector<uint8_t> parseHex(const string &hexStr) {

    vector<uint8_t> bytes;
    stringstream ss(hexStr);
    string byte;

    while (ss >> byte) {
        bytes.push_back(stoul(byte, nullptr, 16));
    }

    return bytes;
}



void writeFeatureRow(ofstream &out) {

    float rpm_mean =
        (T.Wheel_RPM_FL +
         T.Wheel_RPM_FR +
         T.Wheel_RPM_RL +
         T.Wheel_RPM_RR) / 4.0;

    float rpm_std =
        sqrt(
            (pow(T.Wheel_RPM_FL - rpm_mean, 2) +
             pow(T.Wheel_RPM_FR - rpm_mean, 2) +
             pow(T.Wheel_RPM_RL - rpm_mean, 2) +
             pow(T.Wheel_RPM_RR - rpm_mean, 2)) / 4.0
        );

    float Battery_Power_W =
        T.Battery_Voltage_V * T.Battery_Current_A;

    float Power_per_Speed =
        Battery_Power_W / (T.Vehicle_Speed_kmh + 0.001);

    float Target_Thr_mean =
        (T.Target_Thr_FL +
         T.Target_Thr_FR +
         T.Target_Thr_RL +
         T.Target_Thr_RR) / 4.0;

    float Torque_Command_Error =
        fabs(Target_Thr_mean - T.Throttle_Pct);

    float Brake_Pressure_Diff =
        fabs(T.Brake_Pressure_Front_kPa -
             T.Brake_Pressure_Rear_kPa);

    float Temp_Stress_Index =
        T.Battery_Temp_Max_C *
        fabs(T.Battery_Current_A);


    out << T.Vehicle_Speed_kmh << ","
        << rpm_mean << ","
        << rpm_std << ","
        << T.Battery_Voltage_V << ","
        << T.Battery_Current_A << ","
        << Battery_Power_W << ","
        << Power_per_Speed << ","
        << T.Battery_SOC_Pct << ","
        << T.Battery_Resistance_Ohms << ","
        << T.Battery_Temp_Max_C << ","
        << Temp_Stress_Index << ","
        << T.Throttle_Pct << ","
        << Torque_Command_Error << ","
        << T.Brake_Pedal_Pct << ","
        << T.Brake_Pressure_Front_kPa << ","
        << T.Brake_Pressure_Rear_kPa << ","
        << Brake_Pressure_Diff
        << "\n";
}



void decodeCAN(CAN_Message &msg, ofstream &out) {

    uint8_t *d = msg.data;

    switch (msg.identifier) {

        case 0x20:
            T.Throttle_Pct = d[0] * 0.4;
            T.Brake_Pressure_Front_kPa = d[2] * 4.0;
            T.Brake_Pressure_Rear_kPa = d[3] * 4.0;
            T.Brake_Pedal_Pct = d[4] * 0.4;
            break;

        case 0x24:
            T.Battery_Voltage_V = getLE(d,0) * 0.1;
            T.Battery_Current_A = -320 + (getLE(d,2) * 0.1);
            T.Battery_SOC_Pct = d[4];
            T.Battery_Resistance_Ohms = getLE(d,5) * 0.000001;
            T.Battery_Temp_Max_C = d[7] - 40.0;
            break;

        case 0x30:
            T.Target_Thr_FL = d[0] * 0.4;
            T.Target_Thr_FR = d[1] * 0.4;
            T.Target_Thr_RL = d[2] * 0.4;
            T.Target_Thr_RR = d[3] * 0.4;
            break;

        case 0x34: T.Wheel_RPM_FL = getLE(d,0) / 30.0; break;
        case 0x35: T.Wheel_RPM_FR = getLE(d,0) / 30.0; break;
        case 0x36: T.Wheel_RPM_RL = getLE(d,0) / 30.0; break;
        case 0x37: T.Wheel_RPM_RR = getLE(d,0) / 30.0; break;

        case 0x38:
            T.Vehicle_Speed_kmh = getLE(d,0) / 256.0;

            writeFeatureRow(out);
            break;
    }
}



int main() {

    ifstream file("can_log_dataset.csv");

    if (!file.is_open()) {
        cout << "Failed to open can_log_dataset.csv\n";
        return 1;
    }

    ofstream out("iforest_features.csv");

    // ===== HEADER =====
    out << "Vehicle_Speed_kmh,"
        << "Vehicle_Speed_RPM_mean,"
        << "Wheel_RPM_std,"
        << "Battery_Voltage_V,"
        << "Battery_Current_A,"
        << "Battery_Power_W,"
        << "Power_per_Speed,"
        << "Battery_SOC_Pct,"
        << "Battery_Resistance_Ohms,"
        << "Battery_Temp_Max_C,"
        << "Temp_Stress_Index,"
        << "Throttle_Pct,"
        << "Torque_Command_Error,"
        << "Brake_Pedal_Pct,"
        << "Brake_Pressure_Front_kPa,"
        << "Brake_Pressure_Rear_kPa,"
        << "Brake_Pressure_Diff\n";

    string line;
    getline(file, line); 

    while (getline(file, line)) {

        stringstream ss(line);
        vector<string> cols;
        string cell;

        while (getline(ss, cell, ',')) {
            cols.push_back(cell);
        }

        if (cols.size() < 8) continue;

        string idStr = cols[5];    
        string dataStr = cols[7]; 

        uint32_t id = stoul(idStr, nullptr, 16);
        vector<uint8_t> data = parseHex(dataStr);

        CAN_Message msg;
        msg.identifier = id;

        for (size_t i = 0; i < data.size() && i < 8; i++)
            msg.data[i] = data[i];

        decodeCAN(msg, out);
    }

    cout << "Feature dataset saved to iforest_features.csv\n";

    return 0;
}