#include<SlaveSPIClass.h>
 int SlaveSPI::size = 0;
 SlaveSPI** SlaveSPI::SlaveSPIVector = NULL;
void setupIntr(spi_slave_transaction_t * trans)
{
	for(int i=0 ; i<SlaveSPI::size;i++)
	{
		if(SlaveSPI::SlaveSPIVector[i]->match(trans))
				SlaveSPI::SlaveSPIVector[i]->setup_intr(trans);
	}
}
void transIntr(spi_slave_transaction_t * trans)
{
	for(int i=0 ; i<SlaveSPI::size;i++)
	{
		if(SlaveSPI::SlaveSPIVector[i]->match(trans))
				SlaveSPI::SlaveSPIVector[i]->trans_intr(trans);
	}
}
SlaveSPI::SlaveSPI()
{
	SlaveSPI** temp = new SlaveSPI * [size+1];
	for (int i=0;i<size;i++)
	{
		temp[i]=SlaveSPIVector[i];
	}
	temp[size] = this;
	size++;
	delete [] SlaveSPIVector;
	SlaveSPIVector = temp;
	buff = "";
	transBuffer = "";
}
void SlaveSPI::begin(gpio_num_t so,gpio_num_t si,gpio_num_t sclk,gpio_num_t ss,size_t length,void(* ext)())
{
	
	
	//should set to the minimum transaction length
	t_size = length;
	
	driver = new spi_slave_transaction_t{t_size * 8 ,	0 ,	heap_caps_malloc(max(t_size,32), MALLOC_CAP_DMA),	heap_caps_malloc(max(t_size,32), MALLOC_CAP_DMA),NULL};
	
	spi_bus_config_t buscfg={
        .mosi_io_num=si,
        .miso_io_num=so,
        .sclk_io_num=sclk
    };
	
    gpio_set_pull_mode(sclk, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(ss, GPIO_PULLUP_ONLY);
	spi_slave_interface_config_t slvcfg={ss,0,1,0,setupIntr,transIntr};//check the IDF for further explenation
	
	spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg,1); //DMA channel 1
    
	spi_slave_queue_trans(HSPI_HOST,driver,portMAX_DELAY);//ready for input (no transmit)
	
	exter_intr =ext;
}

void SlaveSPI::setup_intr(spi_slave_transaction_t *trans)
{//called when the trans is set in the queue
	//didn't find use for it in the end.
}
void SlaveSPI::trans_intr(spi_slave_transaction_t *trans)
{//called when the trans has finished

	for(int i=0;i<t_size;i++)
	{
		buff += ((char*)driver->rx_buffer)[i];
		((char*) driver->rx_buffer)[i] = (char)0;
	}
	setDriver();
	exter_intr();
}

void SlaveSPI::trans_queue(String& transmission) {
  //used to queue data to transmit
	for (int i=0;i<transmission.length();i++)
		transBuffer += transmission[i];
}

void SlaveSPI::trans_queue(uint8_t *buf, int len) {
  //used to queue data to transmit
	for (int i=0; i < len; i++) {
		transBuffer += buf[i];
	}
}

inline bool SlaveSPI::match(spi_slave_transaction_t * trans)
{
	return (this->driver == trans);
}
void SlaveSPI::setDriver()
{
	driver->user = NULL;
	int i=0;
	for(;i<t_size&&i<transBuffer.length();i++)
	{
		((char*) driver->tx_buffer)[i] = transBuffer[i];
		
	}
	transBuffer = &(transBuffer[i]);
	driver->length=t_size*8;
	driver->trans_len = 0;
	spi_slave_queue_trans(HSPI_HOST,driver,portMAX_DELAY);
}
char SlaveSPI::read()
{
	char temp = buff[0];
	buff.remove(0,1);
	return temp;
}

