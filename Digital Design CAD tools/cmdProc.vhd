----------------------------------------------------------------------------------
-- Company:  University of Bristol, Engineering department
-- Engineer: Ziyang Liu, Lingzhi Xie
-- 
-- Create Date: 2020/03/03 06:54:18
-- Module Name: command processor - Behavioral
-- Project Name: Digital design, Group project
-- Target Devices: 
-- Tool Versions: 2018.3
----------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.common_pack.all;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;


entity cmdProc is
    port(
	  clk: in std_logic;
      reset: in std_logic;
      rxnow: in std_logic; -- if data from Rx is valid
      rxData: in std_logic_vector (7 downto 0);
      txData: out std_logic_vector (7 downto 0);
      rxdone: out std_logic;
      ovErr:  in std_logic;
      framErr: in std_logic;
      txnow: out std_logic; --if output data port of Tx is valid
      txdone: in std_logic;
      start: out std_logic;
      numWords_bcd: out BCD_ARRAY_TYPE(2 downto 0);
      dataReady: in std_logic;
      byte: in std_logic_vector(7 downto 0);
      maxIndex: in BCD_ARRAY_TYPE(2 downto 0);
      dataResults: in CHAR_ARRAY_TYPE(0 to RESULT_BYTE_NUM-1);
      seqDone: in std_logic
  );
end cmdProc;

architecture Behavioral of cmdProc is
    type state_type is (Initialize, IDLE, DETECT_A, DETECT_RANGE_FOR_Ns, S2, S3, S4, OUTPUT_PEAK_VALUE, OUTPUT_LIST_VALUE, S7, DESTINATION, LOAD_VALUE, OUTPUT_MAX_INDEX);
    signal currentState, nextState: state_type;
    signal counter_N, COUNTER_BYTE, INTEGER_STORED, TOTAL, maxindex_counter : integer range 0 to 999;
    signal BCD : BCD_ARRAY_TYPE(2 downto 0);
    signal MAX_INDEX_8_bit: std_logic_vector(7 downto 0);
    signal start_list_values ,output_peak_values, output_maxindex, RESET_maxindex_counter: std_logic;
    signal RESETcounter, enable_counter, list_resetcounter, list_enable, resetBCD, RESET_BYTE_COUNTER, ENABLE_BYTE_COUNTER: std_logic;
    signal list_counter : integer range 0 to 7;
    signal DATARESULTS_SIGNAL : CHAR_ARRAY_TYPE(0 to RESULT_BYTE_NUM-1);
    signal MAXINDEX_SIGNAL: BCD_ARRAY_TYPE(2 downto 0);
BEGIN

----------------------------------------------------------------------
-- State_logic process defined how our state changes as different condition has met.
----------------------------------------------------------------------
state_logic: process(ovErr, framErr, nextState, currentState,rxData, rxnow, txdone , 
RESETcounter, enable_counter, counter_N, seqDone, dataresults, maxIndex, 
start_list_values, output_peak_values, output_maxindex, resetBCD, list_resetcounter, RESETcounter,
enable_counter)
BEGIN

    case currentState is 
    
        WHEN Initialize =>  -- Fist Global reset, it will reset all the signals in this process, although this part is not neccessary
            if reset = '1' then
                start_list_values <= '0';   -- For L command
                output_peak_values <= '0';  -- For P command
                output_maxindex <= '0';     -- For P command, output max index from data processor
                resetBCD <= '1';            -- Reset the BCD counter, (detection sequence of ANNN)
                list_resetcounter <= '1';   -- List counter used to count 7 bytes data from data processor
                list_enable <= '0';
                RESETcounter <= '1';        -- Reset the counter, note: this counter is used to count 3 Ns for BCD output
                enable_counter <= '0';      -- Reset the enable signal to zero, it is also part of BCD counter
                nextState <= Initialize;
            elsif start_list_values = '0'and output_peak_values = '0' and output_maxindex = '0' and resetBCD = '1' and 
            list_resetcounter='1' and RESETcounter='1' and enable_counter = '0' then
                nextState <= IDLE;  -- jump to IDLE  
            else
                nextState <= Initialize;
            end if;
            
        WHEN IDLE =>    
            if rxnow = '1'  THEN -- First Rxdata detected, transmission starts
                nextState <= DETECT_A;
            else                 -- if there is no Rxdata, remains at IDLE, until first rxData arrvied
                nextState <= IDLE;
            end if;
               
        WHEN DETECT_A =>
             if (rxData = "01100001" or rxData = "01000001") and rxnow = '0' and txdone = '1' THEN 
             -- distingiush 'A' or 'a' from ASCII Table, and rxnow and txdone is used for control the flow of data
                    nextState <= LOAD_VALUE;
             end if;
             
        WHEN LOAD_VALUE => -- AFTER detected A and now start count Ns 
            enable_counter <= '0';
            if counter_N = 0 THEN -- down counter count from 2 to 0, if it count 3 times, it will jump to S2.
                nextState <= S2;
            end if;
            if rxnow = '1' and txdone = '1' THEN -- if new rxdata received go to DETECT_RANGE_FOR_Ns
                                                 -- state for make sure N is bewteen 0 ~ 9 ASCII code
                nextState <= DETECT_RANGE_FOR_Ns;
            end if;
       
        WHEN DETECT_RANGE_FOR_Ns => -- detection for ASCII range in Ns (ANNN)
            resetBCD <= '0';   
            if (rxData < "00111010" and rxData > "00101111") and rxnow = '0' and txdone = '1' THEN -- IF Ns are in 0 ~ 9 integer range
                RESETcounter <= '0';
                if counter_N > 0 THEN -- if down counter is not zero, it will count once
                    RESETcounter <= '0';
                    enable_counter <= '1';
                    nextState <= LOAD_VALUE;     
                end if;
            elsif rxData > "00111010" or RxData < "00101111" THEN -- if rxData is not in range 0 ~ 9 ASCII code, jump to IDLE
                RESETcounter <= '1';
                nextState <= IDLE;
            end if;
          
        WHEN S2 =>
            resetBCD <= '1';  -- RESET the BCD counter to transform the Rxdata to Nums_of_BCD array type data, transmit to data processor
            nextState <= S3;
          
        WHEN S3 =>
            DATARESULTS_SIGNAL <= dataresults; -- store the dataresults output from data processor
            MAXINDEX_SIGNAL <= maxIndex; -- store the maxindex output from data processor
            if seqDone = '1' THEN -- make sure sequence is done
                nextState <= S4;
            end if;
            
        WHEN S4 =>
            RESETcounter <= '1'; -- reset the counter for Ns(ANNN) if A or a occurs
            if (rxData = ("01010000") or rxData = ("01100000")) and rxnow = '0' and txdone = '1' THEN --detec P or p
                nextState <= OUTPUT_PEAK_VALUE;
            elsif (rxData = ("01001100") or rxData = ("01101100")) and rxnow = '0' and txdone = '1' THEN -- detect L or l
                 nextState <= OUTPUT_LIST_VALUE;
            elsif (rxData = ("01100001") or rxData = ("01000001")) and rxnow = '0' and txdone = '1' THEN -- detect a or A
                 nextState <= LOAD_VALUE; -- BACK TO S1
            else
                nextState <= S4;
            end if;
        
        WHEN OUTPUT_PEAK_VALUE =>
            output_peak_values <= '1'; -- output the peak values
            nextState <= OUTPUT_MAX_INDEX;
            
        WHEN OUTPUT_MAX_INDEX =>
            output_maxindex <= '1'; -- after output the peak values we should print out maxindex
            if maxindex_counter = 3 THEN -- maxindex involves 3 numbers, we need to print it out in 8-bit data format
                nextState <= DESTINATION;
            else 
                nextState <= OUTPUT_MAX_INDEX;
            end if;
            
        WHEN OUTPUT_LIST_VALUE =>  -- L command received, so we should list all 7 bytes data from data processor
            list_resetcounter <= '0';
            start_list_values <= '1';
            list_enable <= '1';
            if list_counter = 7 THEN -- count 7 times
                nextState <= DESTINATION;
            end if;
        
        WHEN DESTINATION =>
            nextState <= IDLE; -- go back to IDLE
            
        WHEN OTHERS => 
            nextState <= IDLE; -- other situations
    end case;
end process;


STATE_FLIPFLOPS : PROCESS(clk, reset) -- Process used to control clock cycle between each states
begin
    if (reset = '1') then
        currentState <= IDLE;
    elsif rising_edge (clk) then
        currentState <= nextState;
    end if;	
end process;

OUTPUT_MAX_INDEX_TRANSFORMATION : PROCESS(clk, reset, RESET_maxindex_counter, MAX_INDEX_8_bit, maxindex_counter)
BEGIN
-- This process is used to transform maxindex signal from data processor to 8-bit data, in order to transmit it to txdata.
    if reset = '1' then
        RESET_maxindex_counter <= '1';
        MAX_INDEX_8_bit <= X"00";
    elsif RESET_maxindex_counter = '1' or reset = '1' THEN
        maxindex_counter <= 0;
        MAX_INDEX_8_bit <= X"00";
    elsif rising_edge(clk) THEN --  Transformation between 8-bit ASCII code and BCD array type
        if output_maxindex = '1' THEN
            if MAXINDEX_SIGNAL(maxindex_counter) = X"0" THEN
                MAX_INDEX_8_bit <= "00110000";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"1" THEN
                MAX_INDEX_8_bit <= "00110001";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"2" THEN
                MAX_INDEX_8_bit <= "00110010";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"3" THEN
                MAX_INDEX_8_bit <= "00110011";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"4" THEN
                MAX_INDEX_8_bit <= "00110100";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"5" THEN
                MAX_INDEX_8_bit <= "00110101";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"6" THEN
                MAX_INDEX_8_bit <= "00110110";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"7" THEN
                MAX_INDEX_8_bit <= "00110111";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"8" THEN
                MAX_INDEX_8_bit <= "00111000";
            elsif MAXINDEX_SIGNAL(maxindex_counter) = X"9" THEN
                MAX_INDEX_8_bit <= "00111001";
            end if;
            maxindex_counter <= maxindex_counter + 1; -- count once
        end if;  
    end if;    

END PROCESS;

LIST_OUTPUT_COUNTER : PROCESS(clk, reset, list_resetcounter, list_counter, list_enable)
BEGIN
-- Counter Process for L command, used to count 7 bytes data from data processor
    if (reset = '1') or list_resetcounter = '1' THEN
        list_counter <= 0;
    elsif rising_edge(clk) THEN
        if list_enable = '1' THEN
            list_counter <= list_counter + 1;
        end if;
    end if;
END PROCESS;

TX_DATA_OUTPUT : PROCESS(clk, reset, rxdata, rxnow, byte, dataResults_signal)
BEGIN
--This process illustrates how out txData port choose data between rxData and Bytes signal

    if reset = '1' THEN -- reset the rxdone and txdata, and also start signal
        rxdone <= '0';
        txData <= X"00";
        start <= '0';
    elsif rising_edge(clk) THEN
        if seqdone = '1' THEN -- if sequence is done and start signal goes low
            start <= '0';
        elsif rxNow = '1' and txdone = '1' THEN -- Rxdata to Txdata
            txData <= RxData;
            rxdone <= '1';
        elsif list_enable = '1' THEN -- print out the dataresults as L commands followed
            txData <= dataResults_signal(list_counter);
            rxdone <= '1';
        elsif output_peak_values = '1' THEN -- output the peak values which is forth number in dataresults lists(7 length)
            txData <= dataResults_signal(3);
            rxdone <= '1';
        elsif output_maxindex = '1' THEN -- output the max index in a 8-bit format
            txData <= MAX_INDEX_8_bit;
            rxdone <= '1';
        elsif dataready = '1' THEN -- dataready then we could output the byte data 
            txData <= byte;
            rxdone <= '1';
            start <= '0';
        elsif counter_N = 0 and seqdone = '0' THEN
            start <= '1';
        elsif rxnow = '0' THEN -- control rxdone
            rxdone <= '0';
        end if; 
    end if;
END PROCESS;

TX_MODULE_OUTPUT : PROCESS(clk, reset) -- this process control the TxNow output, control the request for Txdone to be low
BEGIN
    if reset = '1' THEN
        txnow <= '0';
    elsif rising_edge(clk) THEN
        if txdone = '1' and rxnow = '1' THEN
            txnow <=  '1';
        elsif txdone = '0' THEN
            txnow <=  '0';
        end if;
   end if;
END PROCESS;


BYTE_COUNTER_LOGIC : PROCESS(clk, reset, RESET_BYTE_COUNTER, ENABLE_BYTE_COUNTER, COUNTER_BYTE)
BEGIN
-- this process is not necessary, but this process is really helpful to show what errors we could made, it helped us to
-- debug, which could be a very useful process
-- also used to check how many number of bytes had printed, so we could check it very efficiently.
    if reset = '1' then 
        RESET_BYTE_COUNTER <= '1';
        ENABLE_BYTE_COUNTER <= '0';
    elsif RESET_BYTE_COUNTER = '1' THEN
        COUNTER_BYTE <= 0;
    elsif rising_edge(clk) and ENABLE_BYTE_COUNTER = '1' THEN
        COUNTER_BYTE <= COUNTER_BYTE + 1;
        
    end if;
         
END PROCESS;



DOWN_COUNTER : PROCESS(clk, reset, BCD, counter_N, TOTAL, INTEGER_STORED)
BEGIN
-- down counter used for count 3 Ns at the very beginning, 3Ns in ANNN sequence.
    if (reset = '1') or RESETcounter = '1' THEN -- global reset
        counter_N <= 3;
        INTEGER_STORED <= 0;
    elsif resetBCD = '1' THEN -- reset BCD signals
        BCD <= (X"0", X"0", X"0");
        TOTAL <= 0;
    elsif rising_edge(clk) THEN
        if enable_counter = '1' THEN
            counter_N <= counter_N - 1;
            if rxData = "00110000" THEN  --  Transformation between 8-bit ASCII code and BCD array type
                BCD(counter_N-1) <= X"0";
                INTEGER_STORED <= 0 * (10 ** counter_N);
            elsif rxData = "00110001" THEN
                BCD(counter_N-1) <= X"1";
                INTEGER_STORED <= 1 * (10 ** counter_N);
            elsif rxData = "00110010" THEN
                BCD(counter_N-1) <= X"2";
                INTEGER_STORED <= 2 * (10 ** counter_N);
            elsif rxData = "00110011" THEN
                BCD(counter_N-1) <= X"3";
                INTEGER_STORED <= 3 * (10 ** counter_N);
            elsif rxData = "00110100" THEN
                BCD(counter_N-1) <= X"4";
                INTEGER_STORED <= 4 * (10 ** counter_N);
            elsif rxData = "00110101" THEN
                BCD(counter_N-1) <= X"5";
                INTEGER_STORED <= 5 * (10 ** counter_N);
            elsif rxData = "00110110" THEN
                BCD(counter_N-1) <= X"6";
                INTEGER_STORED <= 6 * (10 ** counter_N);
            elsif rxData = "00110111" THEN
                BCD(counter_N-1) <= X"7";
                INTEGER_STORED <= 7 * (10 ** counter_N);
            elsif rxData = "00111000" THEN
                BCD(counter_N-1) <= X"8"; 
                INTEGER_STORED <= 8 * (10 ** counter_N);
            elsif rxData = "00111001" THEN
                BCD(counter_N-1) <= X"9"; 
                INTEGER_STORED <= 9 * (10 ** counter_N);
            end if;
            
            TOTAL <= TOTAL + INTEGER_STORED; -- record the TOTAL number, and will convert it into 8-bit output at later stage.
         elsif counter_N = 0 THEN
            numWords_bcd <= BCD; -- if counter stops, numWords_bcd should be asserted.
        end if;
    end if;
END PROCESS;


end Behavioral;
