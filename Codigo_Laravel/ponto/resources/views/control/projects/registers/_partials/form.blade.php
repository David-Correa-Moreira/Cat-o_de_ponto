@include('control.includes.alerts')

<div class="form-row">
    <div class="form-group col-md-4">
      <label>Primeiro Nome</label>
      <input type="text" class="form-control" placeholder="Nome" name="nome" value="{{ $user->nome ?? '' }}">
    </div>
    <div class="form-group col-md-4">
      <label>Último Nome</label>
      <input type="text" class="form-control" placeholder="Sobrenome" name="sobrenome" value="{{ $user->sobrenome ?? '' }}">
    </div>
    <div class="form-group col-md2">
      <label>Situação?</label>
      <select name='status' class="form-control">
          @if(isset($user))
        {{-- <option value="0">{{ $user->s ? 'Ativo':'Inativo' }}</option> --}}
            <option value="0" {{ $user->s == 0 ? 'selected': '' }}>Inativo</option>
            <option value="1" {{ $user->s == 1 ? 'selected': '' }}>Ativo</option>
          @else
            <option selected value="">-Escolha-</option>
            <option value="0">Inativo</option>
            <option value="1">Ativo</option>
          @endif
      </select>
    </div>
    <div class="form-group col-md-4">
      <label>E-mail</label>
      <input type="email" class="form-control" placeholder="e-mail" name="email" value="{{ $user->email ?? '' }}">
    </div>
    <div class="form-group col-md-4">
      <label>Senha</label>
      <input type="password" class="form-control" placeholder="senha" name="password">
    </div>
  </div>
<button type="submit" class="btn btn-primary">Registrar</button>