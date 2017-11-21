<script type="text/javascript">
	var node = $("#navTree").omTree("getSelected").text;
	var nodeNo = node.substr(3);
	$("#ProgramTableNo").attr('value',nodeNo);
	
	var i ;
	var x = document.getElementById("nGreenSignalRatioID");
	var z;
	for(i = 1 ;  i <= addSplit; i++)
	{
		z = document.createElement("option");
		z.text = splitArray[i-1];
		x.add(z,null);
	}

	x = document.getElementById("nPhaseTurnID");
	for(i = 1 ;  i <= nPhaseTurnCount; i++)
	{
		z = document.createElement("option");
		z.text = phaseTurnArray[i-1];
		x.add(z,null);
	}


</script>
<form traffic='programTable' id="reg">
  <br />
  <h1>方案表</h1>
  <br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft">方案表:&nbsp;&nbsp;</td>
			<td colspan="4">
				<select name="ProgramTableNo" id="ProgramTableNo" class="selectwidth" disabled="disabled" >
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
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">周期长</td>
			<td class="tdpaddingleft">相位差</td>
			<td class="tdpaddingleft">绿信比表</td>
			<td class="tdpaddingleft">相序表</td>
		</tr>
		<tr>
			<td><input type="text" name="nCycleTime" id="nCycleTime" value="0"/></td>
			<td><input type="text" name="nOffset" id="nOffset" value="0"/></td>
			<td><select  id="nGreenSignalRatioID"  class="selectwidth"/></td>
			<td><select  id="nPhaseTurnID"  class="selectwidth"/></td>
		</tr>
	</table>
</form>
