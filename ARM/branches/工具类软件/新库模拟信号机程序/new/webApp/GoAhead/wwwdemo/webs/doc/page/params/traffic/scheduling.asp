<script type="text/javascript">

	var PhaseTurnNode = $("#navTree").omTree("getSelected").text;
    var NodeNo = PhaseTurnNode.substr(3);
    $("#SchedulingNo").attr('value',NodeNo);

	var i ;
	var x = document.getElementById("TimeIntervalNum");
	var z;
	for(i = 1 ;  i <= nTimeInterval; i++)
	{
		z = document.createElement("option");
		z.text = TimeIntervalArray[i-1];
		x.add(z,null);
	}


	function isCheckedAll(val){
		
		if(val == 0)
		{
			if(!$("#Month13").is(":checked"))
			{
				$("#Month13").attr('checked', false);
				for(Num=1;Num<=12;Num++)
				{
					MDW = "#Month" + Num;
					$(MDW).attr('checked', false);
				}
			}
			else
			{
				$("#Month13").attr('checked', true);
				for(Num=1;Num<=12;Num++)
				{
					MDW = "#Month" + Num;
					$(MDW).attr('checked', true);
				}
			}//月份		
		}
		else if(val == 1)
		{
			if(!$("#Day32").is(":checked"))
			{
				$("#Day32").attr('checked', false);
				for(Num=1;Num<=31;Num++)
				{
					MDW = "#Day" + Num;
					$(MDW).attr('checked', false);
				}
			}
			else
			{
				$("#Day32").attr('checked', true);
				for(Num=1;Num<=31;Num++)
				{
					MDW = "#Day" + Num;
					$(MDW).attr('checked', true);
				}
			}//日期		
		}
		else if(val == 2)
		{
			if(!$("#WeekDay8").is(":checked"))
			{
				$("#WeekDay8").attr('checked', false);
				for(Num=1;Num<=8;Num++)
				{
					MDW = "#WeekDay" + Num;
					$(MDW).attr('checked', false);
				}
			}
			else
			{
				$("#WeekDay8").attr('checked', true);
				for(Num=1;Num<=8;Num++)
				{
					MDW = "#WeekDay" + Num;
					$(MDW).attr('checked', true);
				}
			}//星期		
		}

	}
</script>

<form traffic='scheduling'>
    <br />
    <h1>调度计划</h1>
    <br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft">调度计划号:&nbsp;&nbsp;</td>
			<td colspan="8">
				<select name="SchedulingNo" id="SchedulingNo" class="selectwidth" disabled="disabled">
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
		            <option value="33">33</option>
		            <option value="34">34</option>
		            <option value="35">35</option>
		            <option value="36">36</option>
		            <option value="37">37</option>
		            <option value="38">38</option>
		            <option value="39">39</option>
		            <option value="40">40</option>
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">时段表:</td>
			<td colspan="8">
				<select name="TimeIntervalNum" id="TimeIntervalNum" class="selectwidth">
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="2">月份</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month1" id="Month1"/>1
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month2" id="Month2"/>2
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month3" id="Month3"/>3
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month4" id="Month4"/>4
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month5" id="Month5"/>5
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month6" id="Month6"/>6
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month7" id="Month7"/>7
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month8" id="Month8"/>8
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month9" id="Month9"/>9
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month10" id="Month10"/>10
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month11" id="Month11"/>11
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Month12" id="Month12"/>12
			</td>
			<td class="tdpaddingleft" colspan="4">
				<input type="checkbox" name="Month13" id="Month13" onClick="isCheckedAll(0)"/>全部选择
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="4">日期</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day1" id="Day1"/>1
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day2" id="Day2"/>2
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day3" id="Day3"/>3
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day4" id="Day4"/>4
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day5" id="Day5"/>5
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day6" id="Day6"/>6
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day7" id="Day7"/>7
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day8" id="Day8"/>8
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day9" id="Day9"/>9
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day10" id="Day10"/>10 
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day11" id="Day11"/>11 
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day12" id="Day12"/>12 
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day13" id="Day13"/>13 
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day14" id="Day14"/>14 
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day15" id="Day15"/>15 
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day16" id="Day16"/>16 
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day17" id="Day17"/>17
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day18" id="Day18"/>18
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day19" id="Day19"/>19
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day20" id="Day20"/>20
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day21" id="Day21"/>21
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day22" id="Day22"/>22
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day23" id="Day23"/>23
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day24" id="Day24"/>24
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day25" id="Day25"/>25
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day26" id="Day26"/>26
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day27" id="Day27"/>27
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day28" id="Day28"/>28
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day29" id="Day29"/>29
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day30" id="Day30"/>30
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day31" id="Day31"/>31
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="Day32" id="Day32" onClick="isCheckedAll(1)"/>全部选择
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">星期</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="WeekDay1" id="WeekDay1"/>1
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="WeekDay2" id="WeekDay2"/>2
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="WeekDay3" id="WeekDay3"/>3
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="WeekDay4" id="WeekDay4"/>4
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="WeekDay5" id="WeekDay5"/>5
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="WeekDay6" id="WeekDay6"/>6
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="WeekDay7" id="WeekDay7"/>日
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="WeekDay8" id="WeekDay8" onClick="isCheckedAll(2)"/>全部选择
			</td>
		</tr>
	</table>
</form>
