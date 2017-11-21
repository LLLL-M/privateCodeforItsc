<script type="text/javascript">

    var PhaseTurnNode = $("#navTree").omTree("getSelected").text;
    var NodeNo = PhaseTurnNode.substr(3);
    $("#PhaseTurnNum").attr('value',NodeNo);

	 $(document).ready(function(){
		
		var H2phase;
		var Hphase;
		var Circle;
		
		for(Circle = 1 ; Circle <= 4 ; Circle++)//依次添加4个环 
		{
			var x = document.getElementById('PhaseTurnTable').insertRow(1+Circle);
			
			var y = x.insertCell(0);
			y.innerHTML = "<td>"+"  环 "+Circle+"</td>";

			for(Hphase = 1 ; Hphase <= addPhase ; Hphase++)
			{
				var z = x.insertCell(Hphase);
				var thisID = "Circle_"+Circle+"_"+Hphase;
				z.innerHTML="<select style=width:50px; id="+thisID+"></select>";
				
				for(H2phase=0;H2phase<=addPhase;H2phase++)
				{
				   var obj=document.getElementById(thisID);
				   if(H2phase >= 1)
				   {
						obj.options.add(new Option(phaseArray[H2phase-1],phaseArray[H2phase-1])); 
				   }
				   else
				   {
						obj.options.add(new Option(0,0)); 
				   }
				}		
			
			}
		
		}
	 });

</script>
<form traffic='sequenceTable'>
    <br />
    <h1>相序表</h1>
    <br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo" id="PhaseTurnTable">
    <tr>
      <td>相序表</td>
      <td>
        <select name="PhaseTurnNum" id="PhaseTurnNum" disabled="disabled">
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

	<tr id="sqe0">
		<td  id="nCricleNum">环号</td>
		<td  id="nTurn" colspan="16" >相位序号</td>
	</tr>
	</table>
</form>
