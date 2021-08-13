//locations excludet for ttnmapper-upload

struct Geofence
{
    double lat;
    double lng;
    u_int radius;
};

static Geofence bielefeld[] = {
    {52.01910, 8.53100, 10000}, //Bielefeld
    {52.51704, 13.38886, 15000} //Berlin
    {48.77845, 9.18001, 5000}   //Stuttgart
};
