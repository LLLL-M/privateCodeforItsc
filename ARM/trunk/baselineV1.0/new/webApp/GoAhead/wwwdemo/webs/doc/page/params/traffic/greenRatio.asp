<form traffic='greenRatio'>
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft">绿信比表:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="greenNum" id="greenNum" class="selectwidth" >
					<option value="1">相位1</option>
	               	<option value="2">相位2</option>
					<option value="3">相位3</option>
	               	<option value="4">相位4</option>
	               	<option value="5">相位5</option>
	               	<option value="6">相位6</option>
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">绿信比:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="greenRatioResult" id="greenRatioResult" value="0"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;秒
	    	</td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">模式:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<select name="greenType" id="greenType" class="selectwidth">
					<option value="1">未定义</option>
	               	<option value="2">其他类型</option>
					<option value="3">无</option>
					<option value="4">最小车辆响应</option>
	               	<option value="5">最大车辆响应</option>
	               	<option value="6">行人响应</option>
	               	<option value="7">最大车辆/行人响应</option>
	               	<option value="8">忽略相位</option>
	            </select>
			</td>
	    </tr>
	    <tr>
			<td class="tdpaddingleft">作为协调相位:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="coordinatePhase" id="coordinatePhase" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">作为关键相位:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="keyPhase" id="keyPhase" value="0"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">作为固定相位:</td>
	        <td class="tdpaddingleft" colspan="3">
	            <input type="checkbox" name="fixedPhase" id="fixedPhase" value="0"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">行人放行:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="pedestrianMove" id="pedestrianMove" value="0"/>秒
	    	</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">行人清空:</td>
	        <td class="tdpaddingleft" colspan="3">
	        	<input type="text" name="pedestrianEmpty" id="pedestrianEmpty" value="0"/>秒
	        </td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">临界绿灯:</td>
	        <td class="tdpaddingleft" colspan="3">
	        	<input type="text" name="frontierGreen" id="frontierGreen" value="0"/>秒
	        </td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">清空模式:</td>
	    	<td class="tdpaddingleft">
		    	<select name="emptyType" id="emptyType" class="selectwidth" >
					<option value="1">红闪</option>
		            <option value="2">绿闪</option>
		        </select>
		    </td>
		</tr>
	</table>
</form>
