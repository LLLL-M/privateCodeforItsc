<script type="text/javascript">
    var ChannelNode = $("#navTree").omTree("getSelected").text;
    var NodeNo = ChannelNode.substr(3);
    $("#ActionTable").attr('value',NodeNo);

	var i ;
	var x = document.getElementById("ProgramNo");
	var z;
	for(i = 0 ;  i <= addProgram; i++)
	{
		z = document.createElement("option");
		if(i >= 1)
		{
			z.text = programArray[i-1];
		}
		else
		{
			z.text = 0;
		}
		x.add(z,null);	
	}
	
	
//alert("===>  "+nScheduleCount);
</script>

<form traffic='timeBasedActionTable'>
	<br />
	<h1>时基动作表</h1>
	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft">动作表: </td>
			<td class="tdpaddingleft" colspan="8">
				<select name="ActionTable" id="ActionTable" class="selectwidth" disabled="disabled" >
					<option value="1">1</option>
	               	<option value="2">2</option>
					<option value="3">3</option>
	               	<option value="4">4</option>
	               	<option value="5">5</option>
	               	<option value="6">6</option>
	               	<option value="7">7</option>
	               	<option value="8">8</option>
	               	<option value="9">9</option>
	               	<option value="10">10</option>
	               	<option value="11">11</option>
	               	<option value="12">12</option>
	               	<option value="13">13</option>
	               	<option value="14">14</option>
	               	<option value="15">15</option>
	               	<option value="16">16</option>
	               	<option value="17">17</option>
	               	<option value="18">18</option>
	               	<option value="19">19</option>
	               	<option value="20">20</option>
	               	<option value="21">21</option>
	               	<option value="22">22</option>
	               	<option value="23">23</option>
	               	<option value="24">24</option>
	               	<option value="25">25</option>
	               	<option value="26">26</option>
	               	<option value="27">27</option>
	               	<option value="28">28</option>
	               	<option value="29">29</option>
	               	<option value="30">30</option>
	               	<option value="31">31</option>
	               	<option value="32">32</option>
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">方案: </td>
			<td class="tdpaddingleft" colspan="8">
				<select name="ProgramNo" id="ProgramNo" class="selectwidth" >
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">辅助功能 :</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="AssistFunction1" id="AssistFunction1" value="0"/>1
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="AssistFunction2" id="AssistFunction2" value="0"/>2
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="AssistFunction3" id="AssistFunction3" value="0"/>3
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="AssistFunction4" id="AssistFunction4" value="0"/>辉度调节
	    	</td>
	    </tr>
	    <tr>
			<td class="tdpaddingleft" rowspan="2">特殊功能 :</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="SpecialFunction1" id="SpecialFunction1" value="0"/>1
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="SpecialFunction2" id="SpecialFunction2" value="0"/>2
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="SpecialFunction3" id="SpecialFunction3" value="0"/>3
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="SpecialFunction4" id="SpecialFunction4" value="0"/>4
	    	</td>
	    </tr>
	    <tr>
			<!-- <td class="tdpaddingleft" bgcolor="#999999">&nbsp;</td> -->
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="SpecialFunction5" id="SpecialFunction5" value="0"/>5
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="SpecialFunction6" id="SpecialFunction6" value="0"/>6
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="SpecialFunction7" id="SpecialFunction7" value="0"/>7
	    	</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="SpecialFunction8" id="SpecialFunction8" value="0"/>8
	    	</td>
	    </tr>
	</table>

</form>
