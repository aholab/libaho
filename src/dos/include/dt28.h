/* PCLAB function definitions -- C header include file. */


/* configuration functions */

extern short initialize( void );        /* Uso ‚sta */

extern short select_board( short );     /* Uso ‚sta */

extern short reset_dt( short * );

extern short set_timeout( short );

extern short find_dma_length( unsigned short *, short * );

extern short terminate( void );         /* Uso ‚sta */


/* analog input functions */

extern short adc_value( short, short, unsigned short * );

extern short adc_on_trigger( short, short, unsigned short * );

extern short setup_adc( short, short, short, short ); /* Uso ‚sta */

extern short adc_series( short, unsigned short * );

extern short begin_adc_dma( short, unsigned short * );

extern short continuous_adc_dma( short, unsigned short * );  /* Uso ‚sta */
/* B£ffer circular */

extern short test_adc_dma( short * );            /* Uso ‚sta */
/* Posici¢n del puntero DMA medida desde el final del b£ffer */

extern short wait_adc_dma( unsigned short * );

extern short stop_adc_dma( void );                /* Uso ‚sta */


/* analog output functions */

extern short dac_value( short, unsigned short * );

extern short dac_on_trigger( short, unsigned short * );

extern short setup_dac( short, short );            /* Uso ‚sta */

extern short dac_series( short, unsigned short * );

extern short begin_dac_dma( short, unsigned short * );

extern short continuous_dac_dma( short, unsigned short * );   /* Uso ‚sta */
/* B£ffer circular */

extern short test_dac_dma( short * );                   /* Uso ‚sta */
/* Posici¢n del puntero DMA medida desde el final del b£ffer */

extern short wait_dac_dma( unsigned short * );          /* Uso ‚sta */

extern short stop_dac_dma( void );                       /* Uso ‚sta */


/* digital input and output functions */

extern short enable_for_output( short );

extern short enable_for_input( short );

extern short output_digital_value( short, unsigned short, unsigned short );

extern short output_digital_on_trigger( short, unsigned short, unsigned short);

extern short input_digital_value( short, unsigned short, unsigned short * );

extern short input_digital_on_trigger(short, unsigned short, unsigned short *);


/* clock functions */

extern short set_clock_divider( long );

extern short set_clock_frequency( float * );             /* Uso ‚sta */

extern short set_clock_period( float * );


/* DT2806 clock functions */

extern short xdelay( short, long );

extern short xstrobe( short, long );

extern short wait_on_delay( void );

extern short generate_clock( long );

extern short count_events( void );

extern short read_events( long * );

extern short get_frequency( short, short *, short * );

extern short stop_clocks( void );


/* system clock functions */

extern short disable_system_clock( void );

extern short enable_system_clock( void );


/* voltage and temperature functions */

extern short measure_volts( short, float * );

extern short measure_thermocouple( short, short, float * );

extern short measure_compensation( float * );

extern short volts_to_degrees( short, float *, float * );

extern short degrees_to_volts( short, float *, float * );

extern short analog_to_volts( unsigned short, short, float * );


/* iSBX functions */

extern short isbx_read( short, short, short, unsigned short * );

extern short isbx_write( short, short, short, unsigned short );


/* error processor functions */

extern short set_error_control_word( unsigned short );

extern short get_error_code( short * );

extern short get_dt_error( unsigned short * );


/* End of PCLAB function definitions. */
