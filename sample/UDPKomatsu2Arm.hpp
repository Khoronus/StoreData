#ifndef TODAI_IMPACT_UDPKOMATSU2ARM_HPP__
#define TODAI_IMPACT_UDPKOMATSU2ARM_HPP__


#include <opencv2/opencv.hpp>


// 受信データ(自由にデータを足しても良いが,送信側と受信側で同じ構造体を用いること)
struct {
	double data[48];
} typedef udpdata;

/** @brief Class to connect with the komatsu arm position
*/
class UDPKomatsu2Arm
{
public:

	void convert(const char* buffer, int recv_length)
	{
		if (recv_length > 0) {
			udpdata data;

			// Swap the words
			uint64_t v = 0;
			char tmp[8];
			for (int k = 0; k < 48; ++k) {
				for (int i = 0; i < 8; ++i) {
					tmp[i] = buffer[((k + 1) * 8 - 1) - i];
				}
				memcpy(&v, &tmp[0], sizeof(uint64_t));
				data.data[k] = *reinterpret_cast<double*>(&v);
			}

			// Update the state
			{
				double volt = data.data[2];
				double v = conversion_theta1(volt);
				std::cout << "ch2 volt: " << volt << " deg: " << v * 180 / 3.1415 << std::endl;
				volt = data.data[3];
				v = conversion_theta2(volt);
				std::cout << "ch3 volt: " << volt << " deg: " << v * 180 / 3.1415 << std::endl;
				volt = data.data[4];
				v = conversion_theta3(volt);
				std::cout << "ch4 volt: " << volt << " deg: " << v * 180 / 3.1415 << std::endl;
				volt = data.data[1];
				v = conversion_pan(volt);
				std::cout << "ch1 volt: " << volt << " deg: " << v * 180 / 3.1415 << std::endl;

				volt = data.data[28];
				v = conversion_theta_lower1(volt);
				std::cout << "ch28 volt: " << volt << " deg: " << v * 180 / 3.1415 << std::endl;
				volt = data.data[29];
				v = conversion_theta_lower2(volt);
				std::cout << "ch29 volt: " << volt << " deg: " << v * 180 / 3.1415 << std::endl;
				volt = data.data[30];
				//v = conversion_theta_lower3(volt);
				v = conversion_theta_lower3_v20180509(volt);
				std::cout << "ch30 volt: " << volt << " deg: " << v * 180 / 3.1415 << std::endl;
				volt = data.data[27];
				v = conversion_pan_lower(volt);
				std::cout << "ch27 volt: " << volt << " deg: " << v * 180 / 3.1415 << std::endl;

				//// lower body
				//double count = data.data[40];
				//v = conversion_pan_body_lower(count);
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_LOWER, 1, v);
				////std::cout << "ch40 count: " << count << " deg: " << v << std::endl;
				//// upper body
				//count = data.data[39];
				//// The next line is important it is after the lower body
				//// because the angle is expected to be relative to the lower body
				//v = conversion_pan_body_upper(count) + v;
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_UPPER, 1, v);
				////std::cout << "ch39 count: " << count << " deg: " << v << std::endl;
				////std::cout << "ch39 count: " << count << " deg: " << v << std::endl;

				// 2018-10-19
				//// upper body
				//double count = data.data[39];
				//// The next line is important it is after the lower body
				//// because the angle is expected to be relative to the lower body
				//v = conversion_pan_body_upper(count);
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_UPPER, 1, v);
				//// lower body
				//count = data.data[40];
				//v = v - conversion_pan_body_upper(count);
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_LOWER, 1, v);
				//v = v - conversion_pan_body_lower(count);
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_TRACK, 1, v);

				//// 2018-10-25
				////track
				//v = 0;
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_TRACK, 1, v);
				////lower body
				//double count = data.data[40];
				//v = v - conversion_pan_body_lower(count);
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_LOWER, 1, v);
				//// upper body
				//count = data.data[39];
				//v = v + conversion_pan_body_upper(count);
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_UPPER, 1, v);

				//// 2018-10-26
				//// upper body
				//v = 0;
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_UPPER, 1, v);
				////lower body
				//double count = data.data[39];
				//v = v + conversion_pan_body_upper(count);
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_LOWER, 1, v);
				////track
				//count = data.data[40];
				//v = v - conversion_pan_body_lower(count);
				//if (sensorpose_data_) sensorpose_data_->modify(kIDX_IMU_TRACK, 1, v);

				// 2018-10-30
				// upper body
				v = 0;
				std::cout << "upper body: " << v << std::endl;;
				//lower body
				double count = data.data[39];
				v = v - conversion_pan_body_upper(count);
				std::cout << "lower body: count " << count << " deg:" << v << std::endl;;
				//track
				count = data.data[40];
				v = v + conversion_pan_body_lower(count);
				std::cout << "track: count " << count << " deg:" << v << std::endl;;
			}
		}
	}

	double conversion_theta1(double volt) {
		double s1 = volt / 10 * 330 + 585;
		double a1 = 2.5803;
		double b1 = -0.0765;
		double h1 = 197.266;
		double k1 = 743.89;
		double u = (std::pow(h1, 2) + std::pow(k1, 2) - std::pow(s1, 2)) / (2 * h1 * k1);
		double cos_inv = std::acos(u);
		double theta = (a1 + b1) - cos_inv;
		return theta;
	}

	double conversion_theta2(double volt) {
		double s2 = volt / 10 * 273 + 520.2;
		double a2 = 3.0816;
		double b2 = 0.1686;
		double h2 = 659.28;
		double k2 = 166.829;
		double u = (std::pow(h2, 2) + std::pow(k2, 2) - std::pow(s2, 2)) / (2 * h2 * k2);
		double cos_inv = std::acos(u);
		double theta = (a2 + b2) - cos_inv;
		return theta;
	}

	double conversion_theta3(double volt) {
		double s3 = volt / 10 * 180 + 426.8;
		double a3 = 3.46;
		double b3 = -2.24;
		double h3 = 525.79;
		double k3 = 109.636;
		double u = (std::pow(h3, 2) + std::pow(k3, 2) - std::pow(s3, 2)) / (2 * h3 * k3);
		double cos_inv = std::acos(u);
		double theta = (a3 + b3) - cos_inv;
		return theta;
	}

	double conversion_pan(double volt) {
		double X1 = 759.5;
		double X2 = 62.93;
		double Y1 = 130;
		double Y2 = 425.35;
		double L0_2 = std::pow(X1 - X2, 2) + std::pow(Y2 - Y1, 2);
		double L = 171.6;
		double L_2 = L * L;
		double L0 = sqrt(L0_2);

		double b = atan((X1 - X2) / (Y2 - Y1));
		double s = (volt / 10) * 169 + 628;
		double s_2 = s * s;
		double theta = acos((L0_2 + L_2 - s_2) / (2 * L0 * L)) - b;
		return theta;
	}

	double conversion_pan_lower(double volt) {
		double X1 = 759.5;
		double X2 = 62.93;
		double Y1 = 130;
		double Y2 = 425.35;
		double L0_2 = std::pow(X1 - X2, 2) + std::pow(Y2 - Y1, 2);
		double L = 171.6;
		double L_2 = L * L;
		double L0 = sqrt(L0_2);

		double b = atan((X1 - X2) / (Y2 - Y1));
		double s = (volt - 0.685) * 66.148 + 627;
		double s_2 = s * s;
		double theta = acos((L0_2 + L_2 - s_2) / (2 * L0 * L)) - b;
		return theta;
	}


	double conversion_theta_lower1(double volt) {
		double s1 = (volt - 0.25)*77.014 + 640;
		double a0 = 147.84 * (3.1415 / 180);
		double a1 = 14.168 * (3.1415 / 180);
		double h0 = 197.27;
		double h1 = 813.44;
		double u = (std::pow(h0, 2) + std::pow(h1, 2) - std::pow(s1, 2)) / (2 * h0 * h1);
		double cos_inv = std::acos(u);
		double theta = (a0 + a1) - cos_inv;
		return theta;
	}

	double conversion_theta_lower2(double volt) {
		double s2 = (volt - 0.25)*97.936 + 706;
		double a2 = 30.014 * (3.1415 / 180);
		double a3 = 160.83 * (3.1415 / 180);
		double h2 = 941.79;
		double h3 = 256.42;
		double u = (std::pow(h2, 2) + std::pow(h3, 2) - std::pow(s2, 2)) / (2 * h2 * h3);
		double cos_inv = std::acos(u);
		double theta = (a2 + a3) + cos_inv - 3.1415;
		return theta;
	}

	double conversion_theta_lower3(double volt) {
		double l2 = 965;
		double l3 = 814;
		double h4 = 241.96;
		double h5 = 210;
		double h6 = 90;
		double h7 = 193;
		double h8 = 155;
		double a4 = 76.42 * (3.1415 / 180);
		double a5 = 0;
		double a6 = 51 * (3.1415 / 180);//102 * (3.1415 / 180);

		double k1 = sqrt(std::pow(l2, 2) + std::pow(h6, 2) - 2 * l2 * h6 * cos(a5));
		double b1 = acos((std::pow(l2, 2) + std::pow(k1, 2) - std::pow(h6, 2)) / (2 * l2 * k1));
		double k2 = sqrt(std::pow(h4, 2) + std::pow(k1, 2) - 2 * h4 * k1 * cos(a4 - b1));
		double b2 = acos((std::pow(k1, 2) + std::pow(k2, 2) - std::pow(h4, 2)) / (2 * k1 * k2));

		double s3 = (volt - 0.25)*85.909 + 649;

		double sigma1 = acos((std::pow(k2, 2) + std::pow(h5, 2) - std::pow(s3, 2)) / (2 * k2 * h5));
		double k4 = sqrt(std::pow(h5, 2) + std::pow(h6, 2) + 2 * h5 * h6 * cos(sigma1 - a5 - b1 + b2));

		double sigma2 = acos((std::pow(k4, 2) + std::pow(h6, 2) - std::pow(h5, 2)) / (2 * k4 * h6));

		double sigma3 = acos((std::pow(k4, 2) + std::pow(h8, 2) - std::pow(h7, 2)) / (2 * k4 * h8));

		double theta = a5 + a6 + sigma2 + sigma3 - 3.1415;
		return theta;
	}

	double conversion_theta_lower3_v20180418(double volt) {
		double h5 = 250;
		double h6 = 150.5;
		double h7 = 193;
		double h8 = 124.1;
		double a4 = 76.42 * (3.1415 / 180);
		double a5 = 0;
		double a6 = 102 * (3.1415 / 180);//102 * (3.1415 / 180);
		double c1 = 1.070337;
		double c2 = 2.45035;
		double v1 = 1.525;
		double v2 = 3.689;

		double sigma1 = (volt - v1) / (v2 - v1) * (c2 - c1) + c1;
		double k4 = sqrt(std::pow(h5, 2) + std::pow(h6, 2) + 2 * h5 * h6 * cos(sigma1));
		double sigma2 = std::acos((std::pow(k4, 2) + std::pow(h6, 2) - std::pow(h5, 2)) / (2 * k4 * h6));
		double sigma3 = std::acos((std::pow(k4, 2) + std::pow(h8, 2) - std::pow(h7, 2)) / (2 * k4 * h8));

		double theta = a6 + sigma2 + sigma3 - CV_PI;
		return theta;
	}

	double conversion_theta_lower3_v20180509(double volt) {
		double h5 = 250;
		double h6 = 150.5;
		double h7 = 193;
		double h8 = 124.1;
		double a4 = 76.42 * (3.1415 / 180);
		double a5 = 0;
		double a6 = (102 - 70) * (3.1415 / 180);//102 * (3.1415 / 180);
		double c1 = 1.070337;
		double c2 = 2.45035;
		double v1 = 1.525;
		double v2 = 3.689;
		double beta2 = 0.3731;

		double sigma1 = (volt - v1) / (v2 - v1) * (c2 - c1) + c1;
		double k4 = sqrt(std::pow(h5, 2) + std::pow(h6, 2) + 2 * h5 * h6 * cos(sigma1 + beta2));
		double sigma2 = std::acos((std::pow(k4, 2) + std::pow(h6, 2) - std::pow(h5, 2)) / (2 * k4 * h6));
		double sigma3 = std::acos((std::pow(k4, 2) + std::pow(h8, 2) - std::pow(h7, 2)) / (2 * k4 * h8));

		//std::cout << "sigma1: " << sigma1 << std::endl;
		//std::cout << "k4: " << k4 << std::endl;
		//std::cout << "sigma2: " << sigma2 << std::endl;
		//std::cout << "sigma3: " << sigma3 << std::endl;

		double theta = a6 + sigma2 + sigma3 - CV_PI;
		return theta;
	}

	// 2018-10-19
	//double conversion_pan_body_lower(double count) {
	//	double theta = count * CV_PI * 2 / 9000;
	//	return theta;
	//}
	// 2018-18-25
	double conversion_pan_body_lower(double count)
	{
		double theta = count * CV_PI * 2 / 8946;
		return theta;
	}

	double conversion_pan_body_upper(double count) {
		double theta = count * CV_PI * 2 / 7500;
		return theta;
	}



};

#endif // TODAI_IMPACT_UDPKOMATSU2ARM_HPP__