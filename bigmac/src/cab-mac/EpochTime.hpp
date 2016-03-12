#ifndef EPOCH_T
#define EPOCH_T

class EpochT {
    long int sec;
    long int msec;

public:
    inline EpochT():sec(0),msec(0) {}

    inline EpochT(int isec, int imsec):sec(isec),msec(imsec) {}

    inline EpochT(const std::string & str) {
        std::vector<std::string> temp;
        boost::split(temp, str, boost::is_any_of("%"));
        sec = boost::lexical_cast<uint32_t> (temp[0]);
        msec = boost::lexical_cast<uint32_t> (temp[1]);
    }

    inline EpochT(const double & dtime) {
        sec = int(dtime);
        msec = int((dtime-sec)*1000000);
    }

    inline EpochT(const int & itime) {
        sec = int(itime);
        msec = 0;
    }

    inline EpochT(const EpochT & rhs) {
        sec = rhs.sec;
        msec = rhs.msec;
    }

    /*
    inline EpochT& operator=(const EpochT & rhs){
    	sec = rhs.sec;
    	msec = rhs.msec;
    	return *this;
    }*/

    inline EpochT operator+(const double & dtime) const {
        long sec = this->sec + int(dtime);
        long msec = this->msec + int((dtime-int(dtime))*1000000);
        if (msec > 1000000) {
            msec -= 1000000;
            sec += 1;
        }
        EpochT res(sec, msec);
        return res;
    }

    inline EpochT operator+(const int & rhs) const {
        EpochT res(this->sec+rhs, this->msec);
        return res;
    }

    inline EpochT operator+(const EpochT & rhs) const {
        long sec = this->sec+rhs.sec;
        long msec = this->msec+rhs.sec;
        if (msec > 1000000) {
            msec -= 1000000;
            sec += 1;
        }
        EpochT res(sec, msec);
        return res;
    }

    inline EpochT operator-(const EpochT & rhs) const {
        long sec = this->sec-rhs.sec;
        long msec = this->msec-rhs.sec;
        if (msec < 0) {
            msec += 1000000;
            sec -= 1;
        }
        EpochT res(sec, msec);
        return res;
    }

    bool operator<(const EpochT & rhs) const {
        if (this->sec < rhs.sec) {
            return true;
        }
        else if(this->sec == rhs.sec) {
            if (this->msec < rhs.sec)
                return true;
        }
        return false;
    }

    double toDouble(const EpochT & offset) const {
        double res = this->sec - offset.sec;
        res += double(this->msec - offset.msec)/1000000;
        return res;
    }

};

#endif
