#ifndef SLAVE_SPI_CLASS
#define SLAVE_SPI_CLASS
#include "Arduino.h"
#include "driver/spi_slave.h"
// #include <functional>
#define SPI_BUFFER_LENGTH 34
#define SPI_BUFFER_PACKET_SIZE 32
void setupIntr(spi_slave_transaction_t * trans);
void transIntr(spi_slave_transaction_t * trans);
class SlaveSPI
{
	static SlaveSPI** SlaveSPIVector;
	static int size;
	friend void setupIntr(spi_slave_transaction_t * trans);
	friend void transIntr(spi_slave_transaction_t * trans);
	String buff;//used to save incoming data
	String transBuffer;//used to buffer outgoing data !not tested!
	spi_slave_transaction_t * driver;
	void (*exter_intr)();//interrupt at the end of transmission , if u need to do something at the end of each transmission
	String perfectPrintByteHex(uint8_t b);
	public:
	SlaveSPI();
	uint8_t bufferRx[SPI_BUFFER_LENGTH];
	uint8_t bufferTx[SPI_BUFFER_LENGTH];
	void setup_intr(spi_slave_transaction_t *trans);//called when the trans is set in the queue
	void trans_intr(spi_slave_transaction_t *trans);//called when the trans has finished
	void setStatus(uint8_t status);
	void setData(uint8_t *buf, int len);
	
	void begin(gpio_num_t so,gpio_num_t si,gpio_num_t sclk,gpio_num_t ss,void(* ext)() = NULL);
	void trans_queue(String& transmission);//used to queue data to transmit 
	void trans_queue(uint8_t *buf, int len);
	inline char* operator[](int i){return (&buff[i]);}
	inline void flush(){buff = "";}
	inline bool match(spi_slave_transaction_t * trans);
	void setDriver();
	inline String* getBuff(){return &buff;}

};

#endif