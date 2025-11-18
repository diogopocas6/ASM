void Sensors::begin(uint8_t pObj, uint8_t pDrop){
    _obj = pObj; _drop = pDrop;
    pinMode(_obj,INPUT);
    pinMode(_drop,INPUT);
}
SensorReadings Sensors::read(){
    int o = analogRead(_obj);
    int d = analogRead(_drop);
    return{ o < 400, d < 400}; //Threshold de deteccao de objeto
}
