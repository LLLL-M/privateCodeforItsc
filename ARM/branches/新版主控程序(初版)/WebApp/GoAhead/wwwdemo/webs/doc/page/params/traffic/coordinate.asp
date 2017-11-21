<form traffic='Coordinate'>
  	<br />
  	<h1>协调</h1>
  	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft">控制模式:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="ControlModel" id="ControlModel" class="selectwidth">
					<option value="1">自动</option>
		            <option value="2">手动方案</option>
		            <option value="3">本地感应</option>
		            <option value="4">闪光</option>
		        </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">手动方案:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="ManualMethod" id="ManualMethod" class="selectwidth">
					<option value="1">1</option>
		            <option value="2">2</option>
		            <option value="3">3</option>
		            <option value="4">4</option>
		            <option value="5">5</option>
		            <option value="6">6</option>
		        </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">协调方式:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="CoordinationMode" id="CoordinationMode" class="selectwidth">
					<option value="1">其他</option>
		            <option value="2">驻留等待</option>
		            <option value="3">平滑过度</option>
		            <option value="4">仅增加</option>
		        </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">协调最大方式:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="CoordinateMaxMode" id="CoordinateMaxMode" class="selectwidth">
					<option value="1">其他</option>
		            <option value="2">最大1</option>
		            <option value="3">最大2</option>
		            <option value="4">最大约束</option>
		        </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">协调强制方式:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="CoordinateForceMode" id="CoordinateForceMode" class="selectwidth">
					<option value="1">其他</option>
		            <option value="2">浮动</option>
		            <option value="3">固定</option>
		        </select>
			</td>
		</tr>
	</table>
</form>
