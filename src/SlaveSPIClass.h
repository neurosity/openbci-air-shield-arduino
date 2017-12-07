#ifndef SLAVE_SPI_CLASS
#define SLAVE_SPI_CLASS
#include "Arduino.h"
#include "driver/spi_slave.h"
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
	size_t t_size;//length of transaction buffer, (should be set to maximum transition size)
	
	public:
	SlaveSPI();
	void setup_intr(spi_slave_transaction_t *trans);//called when the trans is set in the queue
	void trans_intr(spi_slave_transaction_t *trans);//called when the trans has finished
	
	void begin(gpio_num_t so,gpio_num_t si,gpio_num_t sclk,gpio_num_t ss,size_t length=128,void(* ext)() = NULL);
	void trans_queue(String& transmission);//used to queue data to transmit 
	inline char* operator[](int i){return (&buff[i]);}
	inline void flush(){buff = "";}
	inline bool match(spi_slave_transaction_t * trans);
	void setDriver();
	char read();
	inline String* getBuff(){return &buff;}

};

#endif